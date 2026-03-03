#import "../include/app_state.h"
#import "../include/process.h"


#include <stdbool.h>
#include <stdlib.h>

struct AppState {
    ProcessArray *pa;     // Los procesos actuales
    pthread_mutex_t lock; // El "semáforo" para que no choquen
    int keep_running;     // Para apagar el programa limpiamente
    SortOrder sort_order;
};

AppState *app_state_create() {
    AppState *as = malloc(sizeof(AppState));
    as->pa = NULL;
    as->keep_running = true;
    as->sort_order = SORT_BY_RAM;
    pthread_mutex_init(&as->lock, NULL);
    return as;
}

void app_state_destroy(AppState *as) {
    proc_array_delete(as->pa);
    pthread_mutex_destroy(&as->lock);
    free(as);
}


// --- Getters & Setters
void app_state_set_data(AppState *as, ProcessArray *new_data) {
    pthread_mutex_lock(&as->lock);
    proc_array_delete(as->pa);
    as->pa = new_data;
    pthread_mutex_unlock(&as->lock);
}

ProcessArray *app_state_get_data(AppState *as) {
    return as->pa;
}

void app_state_lock(AppState *as) {
    pthread_mutex_lock(&as->lock);
}

void app_state_unlock(AppState *as) {
    pthread_mutex_unlock(&as->lock);
}

int app_state_should_run(AppState *as) {
    int tmp;
    pthread_mutex_lock(&as->lock); // Bloqueamos para leer el valor real
    tmp = as->keep_running;
    pthread_mutex_unlock(&as->lock);
    return tmp;
}

void app_state_stop(AppState *as) {
    pthread_mutex_lock(&as->lock);
    as->keep_running = false;
    pthread_mutex_unlock(&as->lock);
}

void app_state_set_sort(AppState *as, SortOrder order) {
    pthread_mutex_lock(&as->lock);
    as->sort_order = order;
    pthread_mutex_unlock(&as->lock);
}

SortOrder app_state_get_sort(AppState *as) {
    pthread_mutex_lock(&as->lock);
    SortOrder s = as->sort_order;
    pthread_mutex_unlock(&as->lock);
    return s;
}
