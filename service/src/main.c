#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <glib.h>
#include <glib-object.h>
#include <luna-service2/lunaservice.h>
#include <pbnjson.h>
#include "service.h"

GMainLoop *gmainLoop;

// This is a deprecated symbol present in meta-lg-webos-ndk but missing in
// latest buildroot NDK. It is required for proper public service registration
// before webOS 3.5.
//
// SECURITY_COMPATIBILITY flag present in CMakeList disables deprecation notices, see:
// https://github.com/webosose/luna-service2/blob/b74b1859372597fcd6f0f7d9dc3f300acbf6ed6c/include/public/luna-service2/lunaservice.h#L49-L53
bool LSRegisterPubPriv(const char* name, LSHandle** sh,
    bool public_bus,
    LSError* lserror) __attribute__((weak));

bool is_elevated()
{
    return (geteuid() == 0);
}

char *hyperiond_cmdline(char *args)
{
    char *tmp = (char *)calloc(1, FILENAME_MAX);
    snprintf(tmp, FILENAME_MAX, "/bin/bash -c 'LD_LIBRARY_PATH=%s OPENSSL_armcap=%i %s/hyperiond %s'", HYPERION_PATH, 0, HYPERION_PATH, args);
    return tmp;
}

char *hyperiond_start_cmdline()
{
    // Run hyperiond in background
    return hyperiond_cmdline("&");
}

char *hyperiond_version_cmdline()
{
    return hyperiond_cmdline("--version");
}

int hyperiond_start(service_t* service)
{
    if (!is_elevated()) {
        return 1;
    } else if (service->running) {
        return 2;
    }

    service->running = true;
    // TODO: system() ftw
    char *command = hyperiond_start_cmdline();
    int res = system(command);

    if (res != 0) {
        service->running = false;
        return 3;
    }

    return 0;
}

int hyperiond_stop(service_t* service)
{
    if (!is_elevated()) {
        return 1;
    } else if (!service->running) {
        return 2;
    }

    // TODO: system() ftw
    system("killall -9 hyperiond");
    service->running = false;

    return 0;
}

int hyperiond_version(service_t* service)
{
    int res = 0;
    // NOTE: --version call is fine even without root privileges
    if (service->hyperiond_version == NULL) {
        service->hyperiond_version = (char *)calloc(FILENAME_MAX, 1);
        if (service->hyperiond_version == NULL) {
            // Buffer allocation failed
            return 1;
        }

        char *command = hyperiond_version_cmdline();
        // Spawn process with read-only pipe
        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            // Opening process failed
            res = 2;
        } else {
            int bytes_read = fread(service->hyperiond_version, 1, FILENAME_MAX, fp);
            if (bytes_read == 0) {
                // Reading process' stdout failed
                res = 3;
            }

            pclose(fp);
        }
    }

    return res;
}

bool service_method_start(LSHandle* sh, LSMessage* msg, void* data)
{
    service_t* service = (service_t*)data;
    LSError lserror;
    LSErrorInit(&lserror);

    jvalue_ref jobj = jobject_create();
    int res = hyperiond_start(service);

    jobject_set(jobj, j_cstr_to_buffer("returnValue"), jboolean_create(res == 0));
    switch (res) {
        case 0:
            jobject_set(jobj, j_cstr_to_buffer("status"), jstring_create("Hyperion.NG started successfully"));
            break;
        case 1:
            jobject_set(jobj, j_cstr_to_buffer("status"), jstring_create("Precondition fail: Not running elevated!"));
            break;
        case 2:
            jobject_set(jobj, j_cstr_to_buffer("status"), jstring_create("Hyperion.NG was already running"));
            break;
        case 3:
            jobject_set(jobj, j_cstr_to_buffer("status"), jstring_create("Hyperion.NG failed to start, reason: Unknown"));
            break;
    }
    LSMessageReply(sh, msg, jvalue_tostring_simple(jobj), &lserror);

    j_release(&jobj);

    return true;
}

bool service_method_stop(LSHandle* sh, LSMessage* msg, void* data)
{
    service_t* service = (service_t*)data;
    LSError lserror;
    LSErrorInit(&lserror);

    jvalue_ref jobj = jobject_create();
    int res = hyperiond_stop(service);
    jobject_set(jobj, j_cstr_to_buffer("returnValue"), jboolean_create(res == 0));
    switch (res) {
        case 0:
            jobject_set(jobj, j_cstr_to_buffer("status"), jstring_create("Hyperion.NG stopped successfully"));
            break;
        case 1:
            jobject_set(jobj, j_cstr_to_buffer("status"), jstring_create("Precondition fail: Not running elevated!"));
            break;
        case 2:
            jobject_set(jobj, j_cstr_to_buffer("status"), jstring_create("Hyperion.NG was not running"));
            break;
    }
    LSMessageReply(sh, msg, jvalue_tostring_simple(jobj), &lserror);

    j_release(&jobj);

    return true;
}

bool service_method_version(LSHandle* sh, LSMessage* msg, void* data)
{
    service_t* service = (service_t*)data;
    LSError lserror;
    LSErrorInit(&lserror);

    jvalue_ref jobj = jobject_create();
    int res = hyperiond_version(service);
    jobject_set(jobj, j_cstr_to_buffer("returnValue"), jboolean_create(res == 0));
    jobject_set(jobj, j_cstr_to_buffer("returnCode"), jnumber_create_i32(res));
    switch (res) {
        case 0:
            jobject_set(jobj, j_cstr_to_buffer("version"), jstring_create(service->hyperiond_version));
            break;
        default:
            jobject_set(jobj, j_cstr_to_buffer("version"), jstring_create("Error while fetching"));
            break;
    }
    LSMessageReply(sh, msg, jvalue_tostring_simple(jobj), &lserror);

    j_release(&jobj);

    return true;
}

bool service_method_status(LSHandle* sh, LSMessage* msg, void* data)
{
    service_t* service = (service_t*)data;
    LSError lserror;
    LSErrorInit(&lserror);

    jvalue_ref jobj = jobject_create();
    jobject_set(jobj, j_cstr_to_buffer("returnValue"), jboolean_create(true));
    jobject_set(jobj, j_cstr_to_buffer("running"), jboolean_create(service->running));
    jobject_set(jobj, j_cstr_to_buffer("elevated"), jboolean_create(is_elevated()));

    LSMessageReply(sh, msg, jvalue_tostring_simple(jobj), &lserror);

    j_release(&jobj);

    return true;
}

bool service_method_terminate(LSHandle* sh, LSMessage* msg, void* data __attribute__((unused)))
{
    LSError lserror;
    LSErrorInit(&lserror);

    jvalue_ref jobj = jobject_create();
    jobject_set(jobj, j_cstr_to_buffer("returnValue"), jboolean_create(true));

    LSMessageReply(sh, msg, jvalue_tostring_simple(jobj), &lserror);

    j_release(&jobj);

    // Stopping mainloop!
    g_main_loop_quit(gmainLoop);

    return true;
}

LSMethod methods[] = {
    {"start", service_method_start, LUNA_METHOD_FLAGS_NONE },
    {"stop", service_method_stop, LUNA_METHOD_FLAGS_NONE },
    {"status", service_method_status, LUNA_METHOD_FLAGS_NONE },
    {"version", service_method_version, LUNA_METHOD_FLAGS_NONE },
    {"terminate", service_method_terminate, LUNA_METHOD_FLAGS_NONE },
    { 0, 0, 0 }
};

int main()
{
    service_t service;
    LSHandle *handle = NULL;
    LSError lserror;

    service.running = FALSE;
    service.hyperiond_version = NULL;

    LSErrorInit(&lserror);

    // create a GMainLoop
    gmainLoop = g_main_loop_new(NULL, FALSE);

    bool registered = false;

    if (&LSRegisterPubPriv != 0) {
        registered = LSRegisterPubPriv(SERVICE_NAME, &handle, true, &lserror);
    } else {
        registered = LSRegister(SERVICE_NAME, &handle, &lserror);
    }

    if (!registered) {
        LSErrorFree(&lserror);
        return -1;
    }

    LSRegisterCategory(handle, "/", methods, NULL, NULL, &lserror);
    LSCategorySetData(handle, "/", &service, &lserror);

    LSGmainAttach(handle, gmainLoop, &lserror);

    // run to check continuously for new events from each of the event sources
    g_main_loop_run(gmainLoop);
    // Decreases the reference count on a GMainLoop object by one
    g_main_loop_unref(gmainLoop);

    return 0;
}
