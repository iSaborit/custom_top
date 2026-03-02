//
// Created by Iker Saborit López on 14/2/26.
//

#include "../include/ui.h"
#include "../include/utils.h"

#include <ncurses.h>
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
    mvprintw(row, 1, "%5d %-15.15s %6.1f%% %6dMB %-10.10s %s",
             p->pid,
             p->user,
             p->cpu_usage,
             p->ram_usage,
             get_state_string(p->state),
             p->title);
}

void draw_layout(const Screen scr, const ProcessArray *processArray) {
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

    proc_array_order(processArray);
    int line = 2;
    int printed = 0;
    for (int i = 0; printed <= scr.y - 4; i++) {
        if (processArray->p[i].is_collapsed) {
            continue;
        };
        printed++;
        printProcess(&processArray->p[i], line++);
    }
}
