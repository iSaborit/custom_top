//
// Created by Iker Saborit López on 14/2/26.
//

#ifndef CUSTOM_TOP_UI_H
#define CUSTOM_TOP_UI_H

#define DEFAULT 1
#define ALERT 2
#define INFO 3
#include "process.h"

typedef struct {
    int x;
    int y;
} Screen;

void setup_colors();
void draw_layout(const Screen scr, const ProcessArray *processArray);

#endif // CUSTOM_TOP_UI_H
