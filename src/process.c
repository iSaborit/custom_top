//
// Created by Iker Saborit López on 8/2/26.
//

#include "../include/process.h"
#include "../include/sysinfo.h"
#include "../include/app_state.h"

#include <libproc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Process {
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
};

struct ProcessArray {
    Process *p;
    int length;
};

struct ProcessIterator {
    const ProcessArray *pa;
    int index;
};

Process *createProcess(const int pid, char title[64], const int ram,
                       const State state) {
    Process *p = malloc(sizeof(Process));
    p->pid = pid;
    p->state = state;
    p->ram_usage = ram;
    strcpy(p->title, title);
    return p;
}

Process *proc_alloc(void) { return calloc(1, sizeof(Process)); }

int proc_get_pid(const Process *p) { return p->pid; }
int proc_get_ppid(const Process *p) { return p->ppid; }
int proc_get_gid(const Process *p) { return p->gid; }
int proc_get_is_parent(const Process *p) { return p->is_parent; }
const char *proc_get_user(const Process *p) { return p->user; }
float proc_get_cpu(const Process *p) { return p->cpu_usage; }
int proc_get_ram(const Process *p) { return p->ram_usage; }
State proc_get_state(const Process *p) { return p->state; }
const char *proc_get_title(const Process *p) { return p->title; }
int proc_get_collapsed(const Process *p) { return p->is_collapsed; }
uint64_t proc_get_cpu_snapshot(const Process *p) {
    return p->_cpu_time_snapshot;
}
void proc_set_pid(Process *p, int pid) { p->pid = pid; }
void proc_set_ppid(Process *p, int ppid) { p->ppid = ppid; }
void proc_set_gid(Process *p, int gid) { p->gid = gid; }
void proc_set_is_parent(Process *p, int is_parent) { p->is_parent = is_parent; }
void proc_set_user(Process *p, const char *user) {
    strncpy(p->user, user, sizeof(p->user) - 1);
    p->user[sizeof(p->user) - 1] = '\0';
}
void proc_set_title_from_pid(Process *p, int pid) {
    proc_name(pid, p->title, sizeof(p->title));
}
void proc_set_ram(Process *p, int ram_mb) { p->ram_usage = ram_mb; }
void proc_set_cpu(Process *p, float cpu) { p->cpu_usage = cpu; }
void proc_set_state(Process *p, State state) { p->state = state; }
void proc_set_collapsed(Process *p, int collapsed) {
    p->is_collapsed = collapsed;
}
void proc_set_cpu_snapshot(Process *p, uint64_t snapshot) {
    p->_cpu_time_snapshot = snapshot;
}
void proc_add_cpu(Process *p, float delta) { p->cpu_usage += delta; }
void proc_add_ram(Process *p, int delta) { p->ram_usage += delta; }

static SortOrder _current_sort = SORT_BY_RAM;

static int cmp(const void *a, const void *b) {
    const Process *pa = (const Process *)a;
    const Process *pb = (const Process *)b;
    switch (_current_sort) {
        case SORT_BY_CPU:
            return (pa->cpu_usage < pb->cpu_usage) - (pa->cpu_usage > pb->cpu_usage);
        case SORT_BY_PID:
            return pa->pid - pb->pid;
        case SORT_BY_RAM:
        default:
            return (pa->ram_usage < pb->ram_usage) - (pa->ram_usage > pb->ram_usage);
    }
}

void proc_array_order(const ProcessArray *pa, SortOrder order) {
    _current_sort = order;
    qsort(pa->p, pa->length, sizeof(Process), cmp);
}

ProcessArray *proc_array_create(size_t capacity) {
    ProcessArray *pa = malloc(sizeof(ProcessArray));
    if (!pa)
        return NULL;
    pa->p = calloc(capacity, sizeof(Process));
    if (!pa->p) {
        free(pa);
        return NULL;
    }
    pa->length = 0;
    return pa;
}

Process *proc_array_get(const ProcessArray *pa, size_t i) { return &pa->p[i]; }

int proc_array_length(const ProcessArray *pa) { return pa->length; }

void proc_array_set_length(ProcessArray *pa, int length) {
    pa->length = length;
}

void proc_delete(Process *process) { free(process); }

void proc_array_delete(ProcessArray *processArray) {
    if (!processArray) return;
    free(processArray->p);
    free(processArray);
}

void process_printn(ProcessArray *pa, const int n) {
    if (!pa || !pa->p) {
        printf("Process array is empty or NULL\n");
        return;
    }

    int count = pa->length < n ? pa->length : n;
    printf("Top %d processes:\n", count);
    printf("%-6s %-10s %-30s %-10s %-10s %-10s\n", "PID", "USER", "NAME",
           "RAM (MB)", "CPU (%)", "STATE");
    printf("------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        if (pa->p[i].pid == 0) {
            count++;
            continue;
        }
        printf("%-6d %-10s %-30s %-10d %-10f %-10s\n", pa->p[i].pid,
               pa->p[i].user, pa->p[i].title, pa->p[i].ram_usage,
               pa->p[i].cpu_usage, get_state_string(pa->p[i].state));
    }
}

ProcessIterator *proc_iter_create(const ProcessArray *pa) {
    ProcessIterator *pi = malloc(sizeof(ProcessIterator));
    pi->pa = pa;
    pi->index = 0;

    return pi;
}

const Process *proc_iter_next(ProcessIterator *pi) {
    if (!pi || !pi->pa)
        return NULL;

    while (pi->index < pi->pa->length) {
        const Process *p = &pi->pa->p[pi->index++];

        if (!p->is_collapsed) {
            return p;
        }
    }

    return NULL;
}

void proc_iter_destroy(ProcessIterator *pi) {
    if (pi != NULL) {
        free(pi);
    }
}

void *data_thread_func(void *arg) {
    AppState *as = (AppState *)arg;
    while(app_state_should_run(as)) {
        ProcessArray *pa = get_process_list();

        app_state_set_data(as, pa);
    }

    return NULL;
}
