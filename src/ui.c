//
// Created by Iker Saborit López on 14/2/26.
//

#include "../include/ui.h"
#include "../include/app_state.h"
#include "../include/utils.h"

#include <ncurses.h>
#include <string.h>
#include <unistd.h>

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

static void ui_print_box(const Screen scr, AppState *as) {
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
    mvprintw(0, scr.x - 2 - strlen(message), "%s", message);

    const char *current_query = app_state_get_search(as);

    if (app_state_get_searching_mode(as) ||
        (current_query && strlen(current_query) > 0)) {
        attron(COLOR_PAIR(INFO));
        mvprintw(scr.y - 1, 2, " SEARCH: %s ", current_query);
        attroff(COLOR_PAIR(INFO));
    } else {
        // Si no está buscando, mostramos la leyenda de teclas original
        mvprintw(scr.y - 1, 2, " (P)ID - (C)PU - (R)AM - (/)Search ");
    }

    attroff(COLOR_PAIR(DEFAULT) | A_BOLD);
}

static void draw_layout(const Screen scr, AppState *as, SortOrder order) {
    ui_print_box(scr, as);

    int line = 2;
    int printed = 0;
    const char *filter = app_state_get_search(as);

    ProcessArray *pa = app_state_get_data(as);
    proc_array_order(pa, order);
    ProcessIterator *pi = proc_iter_create(pa);

    const Process *p;
    while ((p = proc_iter_next(pi)) != NULL && printed < scr.y - 3) {
        if (filter && strlen(filter) > 0) {
            if (strstr(proc_get_title(p), filter) == NULL) {
                continue;
            }
        }

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
        } else if (key == 27) {
            app_state_set_search(as, "");
            app_state_set_searching_mode(as, false);
        } else if (key == '/') {
            app_state_set_searching_mode(as, true);
            char buffer[255] = {0};
            int position = 0;

            // Entramos en un sub-bucle de captura de texto
            while (app_state_get_searching_mode(as)) {
                // Mostramos el indicador visualmente en la parte inferior
                // (ejemplo: penúltima línea)
                mvprintw(LINES - 1, 2, "Search: %-50s", buffer);
                clrtoeol(); // Limpia el resto de la línea
                refresh();

                key = getch();

                if (key == 27) { // ESC - Cancelar
                    app_state_set_searching_mode(as, false);
                    app_state_set_search(as, "");
                    break;
                } else if (key == '\n' ||
                           key == KEY_ENTER) { // ENTER - Confirmar
                    app_state_set_searching_mode(as, false);
                    Screen s;
                    getmaxyx(stdscr, s.y, s.x);
                    erase();
                    SortOrder sort = app_state_get_sort(as);
                    ProcessArray *data = app_state_get_data(as);
                    if (data) {
                        draw_layout(s, as, sort);
                    } else {
                        ui_print_box(s, as);
                    }
                    break;
                } else if (key == KEY_BACKSPACE || key == 127 ||
                           key == 8) { // Borrar
                    if (position > 0) {
                        buffer[--position] = '\0';
                        app_state_set_search(as, buffer);
                    }
                } else if (key >= 32 && key <= 126 &&
                           position < 254) { // Caracteres imprimibles
                    buffer[position++] = (char)key;
                    buffer[position] = '\0';
                    app_state_set_search(as, buffer);
                }
                Screen s;
                getmaxyx(stdscr, s.y, s.x);
                erase();
                SortOrder sort = app_state_get_sort(as);
                ProcessArray *data = app_state_get_data(as);
                if (data) {
                    draw_layout(s, as, sort);
                } else {
                    ui_print_box(s, as);
                }
                refresh();
                usleep(10000);
            }

            move(LINES - 1, 0);
            clrtoeol();
        }

        Screen s;
        getmaxyx(stdscr, s.y, s.x);
        erase();
        SortOrder sort = app_state_get_sort(as);
        ProcessArray *data = app_state_get_data(as);
        if (data) {
            draw_layout(s, as, sort);
        } else {
            ui_print_box(s, as);
        }
        refresh();

        usleep(50000);
    }

    return NULL;
}
