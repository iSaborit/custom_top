#include <mach/mach_time.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/process.h"
#include "../include/state.h"
#include "../include/sysinfo.h"
#include "../include/ui.h"

int main(void) {
    initscr();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);

    Screen s;
    getmaxyx(stdscr, s.y, s.x);
    int key = 0;

    setup_colors();

    ProcessArray *pa;
    while (key != 'q' && key != 'Q') {
        pa = get_process_list();
        getmaxyx(stdscr, s.y, s.x);
        erase();
        draw_layout(s, pa);
        refresh();
        key = getch();

        // Liberamos la memoria antes del próximo ciclo para evitar la fuga
        if (key != 'q' && key != 'Q') {
            deleteProcessArray(pa);
        }
    }

    endwin();

    //process_printn(pa, 30);
    deleteProcessArray(pa);
    return 0;
}
