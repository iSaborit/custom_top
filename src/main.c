#include <mach/mach_time.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/ui.h"
#include "../include/app_state.h"

int main(void) {
    initscr(); noecho(); curs_set(0); nodelay(stdscr, TRUE);
    setup_colors();

    AppState *state = app_state_create();

    pthread_t thread_ui, thread_data;
    pthread_create(&thread_data, NULL, data_thread_func, (void *)state);
    pthread_create(&thread_ui, NULL, ui_thread_func, (void *)state);

    pthread_join(thread_ui, NULL);
    pthread_join(thread_data, NULL);

    endwin();
    app_state_destroy(state);

    return 0;
}
