#ifndef CUSTOM_TOP_APPSTATE_H
#define CUSTOM_TOP_APPSTATE_H

#include "process.h"
#include <pthread.h>

typedef struct AppState AppState;

// Funciones de interfaz
AppState *app_state_create();
void app_state_destroy(AppState *as);

// Getters/Setters seguros (con Mutex interno)
void app_state_set_data(AppState *as, ProcessArray *new_data);
ProcessArray *app_state_get_data(AppState *as);
void app_state_lock(AppState *as);
void app_state_unlock(AppState *as);
int app_state_should_run(AppState *as);
void app_state_stop(AppState *as);
void app_state_set_sort(AppState *as, SortOrder order);
SortOrder app_state_get_sort(AppState *as);

#endif // !CUSTOM_TOP_APPSTATE_H
