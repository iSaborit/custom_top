//
// Created by Iker Saborit López on 8/2/26.
//

#include "../include/process.h"

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


Process *createProcess(const int pid, char title[64], const int ram, const State state) {
    Process *p = malloc(sizeof(Process));
    p->pid = pid;
    p->state = state;
    p->ram_usage = ram;
    strcpy(p->title, title);
    return p;
}

int proc_get_pid(const Process *p) {
    return p->pid;
}

const char *proc_get_user(const Process *p) {
    return p->user;
}

float proc_get_cpu(const Process *p){
    return p->cpu_usage;
}

int proc_get_ram(const Process *p){
    return p->ram_usage;
}

State proc_get_state(const Process *p){
    return p->state;
}

const char *proc_get_title(const Process *p){
    return p->title;
}

int proc_get_collapsed(const Process *p) {
    return p->is_collapsed;
}

static int cmp(const void *a, const void *b) {
    const Process *pa = (const Process *)a;
    const Process *pb = (const Process *)b;
    if (pa->ram_usage < pb->ram_usage) return 1;
    else return -1;
}

void proc_array_order(const ProcessArray *pa) {
    qsort(pa->p, pa->length, sizeof(Process), cmp);
}


void proc_delete(Process *process) {
    free(process);
}

void proc_array_delete(ProcessArray *processArray) {
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
    printf("%-6s %-10s %-30s %-10s %-10s %-10s\n", "PID", "USER", "NAME", "RAM (MB)", "CPU (%)", "STATE");
    printf("------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        if (pa->p[i].pid == 0) {
            count++;
            continue;
        }
        printf("%-6d %-10s %-30s %-10d %-10f %-10s\n",
               pa->p[i].pid,
               pa->p[i].user,
               pa->p[i].title,
               pa->p[i].ram_usage,
               pa->p[i].cpu_usage,
               get_state_string(pa->p[i].state));
    }
}
