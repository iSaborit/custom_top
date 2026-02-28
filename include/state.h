//
// Created by Iker Saborit López on 8/2/26.
//

#ifndef CUSTOM_TOP_STATE_H
#define CUSTOM_TOP_STATE_H

typedef enum {
    SLEEPING,
    RUNNING,
    IDLE,
    STOP,
    ZOMBIE,
} State;

const char *get_state_string(State state);

State convert_run_state(int pti_run_state);

#endif //CUSTOM_TOP_STATE_H