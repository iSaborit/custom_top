//
// Created by Iker Saborit López on 14/2/26.
//

#include "../include/ui.h"
#include "../include/app_state.h"
#include "../include/utils.h"

#include <ncurses.h>
#include <unistd.h>
#include <string.h>

void setup_colors() {
    start_color();

    use_default_colors();

    init_pair(DEFAULT, -1, -1);
    init_pair(ALERT, COLOR_RED, COLOR_YELLOW);
    init_pair(INFO, COLOR_YELLOW, -1);
}

static void printProcess(const Process *p, const int row) {
    attron(COLOR_PAIR(DEFAULT));
    mvprintw(row, 1, "%5d %-15.15s %6.1f%% %6dMB %-10.10s %s", proc_get_pid(p),
             proc_get_user(p), proc_get_cpu(p), proc_get_ram(p),
             get_state_string(proc_get_state(p)), proc_get_title(p));
}

static void ui_print_box(const Screen scr) {
    attron(COLOR_PAIR(DEFAULT) | A_BOLD);
    box(stdscr, 0, 0);
    mvprintw(0, 2, "%s", " This is my own htop, but better. (It is not) ");
    char *message = " Press q to exit btw. ";
    mvprintw(0, scr.x - 2 - strlen(message), "%s", message);
    attroff(COLOR_PAIR(DEFAULT) | A_BOLD);

    attron(COLOR_PAIR(DEFAULT) | A_REVERSE | A_BOLD);
    const int width = scr.x - 2;
    char row[width + 1];

    // 1. Construimos la cabecera con anchos exactos
    char header_msg[256];
    snprintf(header_msg, sizeof(header_msg), "%5s %-15s %7s %8s %-10s %s",
             "PID", "USER", "CPU%", "MEM", "STATE", "NAME");

    truncate_string(row, header_msg, width);
    mvprintw(1, 1, "%s", row);
    attroff(COLOR_PAIR(DEFAULT) | A_REVERSE | A_BOLD);

    attron(COLOR_PAIR(DEFAULT) | A_BOLD);
    box(stdscr, 0, 0);
    mvprintw(scr.y - 1, 2, "%s", " (P)ID - (C)PU - (R)AM ");
    mvprintw(0, scr.x - 2 - strlen(message), "%s", message);
    attroff(COLOR_PAIR(DEFAULT) | A_BOLD);
}

static void draw_layout(const Screen scr, const ProcessArray *processArray, SortOrder order) {
    ui_print_box(scr);

    int line = 2;
    int printed = 0;

    proc_array_order(processArray, order);
    ProcessIterator *pi = proc_iter_create(processArray);
    const Process *p;

    while ((p = proc_iter_next(pi)) != NULL && printed < scr.y - 3) {
        printed++;
        printProcess(p, line++);
    }

    proc_iter_destroy(pi);
}

void *ui_thread_func(void *arg) {
    AppState *as = (AppState *)arg;

    int key = 0;
    app_state_set_sort(as, SORT_BY_CPU);

    while (app_state_should_run(as)) {
        key = getch();
        if (key == 'q' || key == 'Q') {
            app_state_stop(as);
        } else if (key == 'r' || key == 'R') {
            app_state_set_sort(as, SORT_BY_RAM);
        } else if (key == 'c' || key == 'C') {
            app_state_set_sort(as, SORT_BY_CPU);
        } else if (key == 'p' || key == 'P') {
            app_state_set_sort(as, SORT_BY_PID);
        }

        Screen s;
        getmaxyx(stdscr, s.y, s.x);
        erase();
        SortOrder sort = app_state_get_sort(as);
        app_state_lock(as);
        ProcessArray *data = app_state_get_data(as);
        if (data) {
            draw_layout(s, data, sort);
        } else {
            ui_print_box(s);
        }
        refresh();
        app_state_unlock(as);

        usleep(500000);
    }

    return NULL;
}
