#pragma once

#include <unistd.h>
#include <glib.h>
#include <pthread.h>

#define LOG_NAME "hyperion.ng-loader"
#define SERVICE_NAME "org.webosbrew.hyperion.ng.loader.service"
#define DAEMON_PATH "/media/developer/apps/usr/palm/services/org.webosbrew.hyperion.ng.loader.service/hyperion"
#define DAEMON_EXECUTABLE "hyperiond"
#define DAEMON_NAME "Hyperion.NG"

typedef struct {
    char *daemon_version;
    pid_t daemon_pid;
    pthread_t execution_thread;
} service_t;

int service_start(service_t* service);
int service_stop(service_t* service);