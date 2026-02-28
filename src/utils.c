//
// Created by Iker Saborit López on 14/2/26.
//

#include "../include/utils.h"

#include <string.h>

void truncate_string(char *container, const char *message, int max_size) {
    if ((unsigned long) max_size < strlen(message)) {
        if (max_size > 3) {
            memcpy(container, message, max_size - 3);
            memcpy(container+max_size-3, "...", 3);
        } else {
            memset(container, '.', max_size);
        }
    } else {
        memset(container, ' ', max_size);
        memcpy(container, message, strlen(message));
    }

    container[max_size] = '\0';
}
