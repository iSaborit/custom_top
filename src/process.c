//
// Created by Iker Saborit López on 8/2/26.
//

#include "../include/process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Process *createProcess(const int pid, char title[64], const int ram, const State state) {
    Process *p = malloc(sizeof(Process));
    p->pid = pid;
    p->state = state;
    p->ram_usage = ram;
    strcpy(p->title, title);
    return p;
}

int cmp(const void *a, const void *b) {
    const Process *pa = (const Process *)a;
    const Process *pb = (const Process *)b;
    if (pa->ram_usage < pb->ram_usage) return 1;
    else return -1;
}

void orderProcessArray(const ProcessArray *pa) {
    qsort(pa->p, pa->length, sizeof(Process), cmp);
}


void deleteProcess(Process *process) {
    free(process);
}

void deleteProcessArray(ProcessArray *processArray) {
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
