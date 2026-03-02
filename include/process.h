//
// Created by Iker Saborit López on 8/2/26.

#ifndef CUSTOM_TOP_PROCESS_H
#define CUSTOM_TOP_PROCESS_H

#include "state.h"
#include <stdint.h>

typedef struct Process Process;
typedef struct ProcessArray ProcessArray;
typedef struct ProcessIterator ProcessIterator;

/*
 *
             p->pid,
             p->user,
             p->cpu_usage,
             p->ram_usage,
             get_state_string(p->state),
             p->title);
 *
 *
 * */

// --- Process.

Process *createProcess(int, char[64], int, State);
void proc_delete(Process *process);

int proc_get_pid(const Process *p);
const char *proc_get_user(const Process *p);
float proc_get_cpu(const Process *p);
int proc_get_ram(const Process *p);
State proc_get_state(const Process *p);
const char *proc_get_title(const Process *p);
int proc_get_collapsed(const Process *p);

// --- ProcessArray

void proc_array_delete(ProcessArray *processArray);
// I gotta put an enum on how we order the pA
void proc_array_order(const ProcessArray *pa);
// testing purposes
void process_printn(ProcessArray *pa, int n);

// --- ProcessIterator
ProcessIterator *proc_iter_create(const ProcessArray *pa);
const Process *proc_iter_next(ProcessIterator *pi);
void proc_iter_destroy(ProcessIterator *pi);

#endif // CUSTOM_TOP_PROCESS_H
