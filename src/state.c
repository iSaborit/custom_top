//
// Created by Iker Saborit López on 8/2/26.
//

#include <sys/proc.h>
#include "../include/state.h"

const char* get_state_string(const State state) {
    switch (state) {
        case IDLE:    return "Idle";
        case RUNNING: return "Running";
        case SLEEPING:return "Sleeping";
        case ZOMBIE:  return "Zombie";
        case STOP:    return "Stop";
        default:      return "Unknown";
    }
}

State convert_run_state(int pti_run_state) {
    switch (pti_run_state) {
        case 1:
            return IDLE;
        case 2:
            return RUNNING;
        case 3:
            return SLEEPING;
        case 4:
            return STOP;
        case 5:
            return ZOMBIE;
        default:
            return SLEEPING;
    }
}

