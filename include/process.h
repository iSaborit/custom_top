//
// Created by Iker Saborit López on 8/2/26.
//

#ifndef CUSTOM_TOP_PROCESS_H
#define CUSTOM_TOP_PROCESS_H

#include "state.h"
#include <stddef.h>
#include <stdint.h>

typedef struct Process Process;
typedef struct ProcessArray ProcessArray;
typedef struct ProcessIterator ProcessIterator;

// --- Process.

Process *createProcess(int, char[64], int, State);
Process *proc_alloc(void);
void proc_delete(Process *process);

// Getters
int proc_get_pid(const Process *p);
int proc_get_ppid(const Process *p);
int proc_get_gid(const Process *p);
int proc_get_is_parent(const Process *p);
const char *proc_get_user(const Process *p);
float proc_get_cpu(const Process *p);
int proc_get_ram(const Process *p);
State proc_get_state(const Process *p);
const char *proc_get_title(const Process *p);
int proc_get_collapsed(const Process *p);
uint64_t proc_get_cpu_snapshot(const Process *p);

// Setters
void proc_set_pid(Process *p, int pid);
void proc_set_ppid(Process *p, int ppid);
void proc_set_gid(Process *p, int gid);
void proc_set_is_parent(Process *p, int is_parent);
void proc_set_user(Process *p, const char *user);
void proc_set_title_from_pid(Process *p, int pid);
void proc_set_ram(Process *p, int ram_mb);
void proc_set_cpu(Process *p, float cpu);
void proc_set_state(Process *p, State state);
void proc_set_collapsed(Process *p, int collapsed);
void proc_set_cpu_snapshot(Process *p, uint64_t snapshot);
void proc_add_cpu(Process *p, float delta);
void proc_add_ram(Process *p, int delta);

// --- SortOrder

typedef enum {
    SORT_BY_RAM,
    SORT_BY_CPU,
    SORT_BY_PID,
} SortOrder;

// --- ProcessArray

ProcessArray *proc_array_create(size_t capacity);
Process *proc_array_get(const ProcessArray *pa, size_t i);
int proc_array_length(const ProcessArray *pa);
void proc_array_set_length(ProcessArray *pa, int length);
void proc_array_delete(ProcessArray *processArray);
void proc_array_order(const ProcessArray *pa, SortOrder order);
void process_printn(ProcessArray *pa, int n);

// --- ProcessIterator
ProcessIterator *proc_iter_create(const ProcessArray *pa);
const Process *proc_iter_next(ProcessIterator *pi);
void proc_iter_destroy(ProcessIterator *pi);

void *data_thread_func(void *arg);

#endif // CUSTOM_TOP_PROCESS_H
