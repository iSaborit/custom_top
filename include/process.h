//
// Created by Iker Saborit López on 8/2/26.

#ifndef CUSTOM_TOP_PROCESS_H
#define CUSTOM_TOP_PROCESS_H

#include <stdint.h>
#include "state.h"

typedef struct {
    int pid;
    int ppid;
    char user[64];
    char title[64];
    uint64_t _cpu_time_snapshot;
    int ram_usage;
    float cpu_usage;
    int is_parent;
    int is_collapsed;
    int gid;
    State state;
} Process;

Process *createProcess(int, char [64], int, State);

void deleteProcess(Process *process);

typedef struct {
    Process *p;
    int length;
} ProcessArray;

void deleteProcessArray(ProcessArray *processArray);

void process_printn(ProcessArray *pa, int n);

void orderProcessArray(const ProcessArray *pa);

#endif // CUSTOM_TOP_PROCESS_H
