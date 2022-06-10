#pragma once

#include <unistd.h>
#include <glib.h>
#include <pthread.h>

#define SERVICE_NAME "org.webosbrew.hyperion.ng.loader.service"
#define HYPERION_PATH "/media/developer/apps/usr/palm/services/org.webosbrew.hyperion.ng.loader.service/hyperion"

typedef struct {
    char *hyperiond_version;
    pid_t daemon_pid;
    pthread_t execution_thread;
} service_t;

int service_start(service_t* service);
int service_stop(service_t* service);