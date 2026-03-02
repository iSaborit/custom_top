#ifndef CUSTOM_TOP_APPSTATE_H
#define CUSTOM_TOP_APPSTATE_H

#include <pthread.h>
#include "process.h"

typedef struct {
    ProcessArray *data;       // Los procesos actuales
    pthread_mutex_t lock;     // El "semáforo" para que no choquen
    bool keep_running;        // Para apagar el programa limpiamente
} AppState;

#endif // !CUSTOM_TOP_APPSTATE_H
