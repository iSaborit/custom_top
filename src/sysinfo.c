#include "../include/sysinfo.h"
#include <libproc.h>
#include <sys/proc.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <mach/mach_time.h>


/*
 * Memory gestion and pids
 */

static int *fetch_pid_list(size_t *out_count) {
    size_t aprox = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0) / sizeof(pid_t);
    size_t size = sizeof(int) * aprox + 4096;

    int *buf = malloc(size);
    if (!buf) {
        perror("Error: malloc pid_list falló");
        return NULL;
    }

    *out_count = (size_t) proc_listpids(PROC_ALL_PIDS, 0, buf, size) / sizeof(int);
    return buf;
}


/*
 * Extract data from MacOS
 */

static int fill_process_static(Process *p, int pid) {
    struct proc_taskinfo task_info;
    int ret = proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &task_info, sizeof(task_info));

    if (ret != sizeof(task_info)) {
        return -1; // No perms
    }

    p->pid = pid;
    p->is_collapsed = false;
    proc_name(pid, p->title, sizeof(p->title));
    p->ram_usage = (int) (task_info.pti_resident_size / 1024 / 1024);

    p->_cpu_time_snapshot = task_info.pti_total_user + task_info.pti_total_system;

    struct proc_bsdinfo bsd;
    ret = proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &bsd, sizeof(bsd));

    if (ret == sizeof(bsd)) {
        p->state = convert_run_state((int) bsd.pbi_status);

        const struct passwd *pwd = getpwuid(bsd.pbi_uid);
        if (pwd) {
            strncpy(p->user, pwd->pw_name, sizeof(p->user) - 1);
            p->user[sizeof(p->user) - 1] = '\0';
        }

        p->gid = bsd.pbi_gid;
        p->ppid = bsd.pbi_ppid;

        if (bsd.pbi_pid == bsd.pbi_gid) {
            p->is_parent = true;
        }
    }

    return 0;
}

static void compute_cpu_usage(Process *p, uint64_t elapsed_ticks) {
    struct proc_taskinfo task_info;
    int ret = proc_pidinfo(p->pid, PROC_PIDTASKINFO, 0, &task_info, sizeof(task_info));

    if (ret != sizeof(task_info)) {
        p->cpu_usage = 0.0f;
        return;
    }

    uint64_t current = task_info.pti_total_user + task_info.pti_total_system;
    uint64_t delta = current - p->_cpu_time_snapshot;

    p->cpu_usage = (float) delta / (float) elapsed_ticks * 100.0f;
}


/*
 * Collapsing process
 */

static void add_process_data(Process *parent, Process *child) {
    parent->cpu_usage += child->cpu_usage;
    parent->ram_usage += child->ram_usage;
}

static void collapse_on_parent(Process *p_list, size_t length, Process *p) {
    for (size_t i = 0; i < length; i++) {
        if (p_list[i].pid == p->ppid) {
            add_process_data(&p_list[i], p);
            p->is_collapsed = true;
            break;
        }
    }
}

static void process_grouping_pass(Process *p_list, size_t length) {
    for (size_t i = 0; i < length; i++) {
        // Protegemos el colapso: Solo agrupamos si el padre NO es launchd (1) ni kernel_task (0)
        if (!p_list[i].is_parent && p_list[i].ppid > 1) {
            collapse_on_parent(p_list, length, &p_list[i]);
        }
    }
}


/*
 * Public API
 */

ProcessArray *get_process_list(void) {
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    // Obtain PID list
    size_t number_of_pids;
    int *pid_list = fetch_pid_list(&number_of_pids);
    if (!pid_list) exit(1);

    // Allocating process array
    Process *p = calloc(number_of_pids, sizeof(Process));
    if (!p) {
        perror("Error: calloc process array falló");
        free(pid_list);
        exit(1);
    }

    // static info + cpu snapshot
    size_t valid = 0;
    uint64_t t_start = mach_absolute_time();

    for (size_t i = 0; i < number_of_pids; i++) {
        if (fill_process_static(&p[valid], pid_list[i]) == 0) {
            valid++;
        }
    }

    // should refactor this bc blocks the ui (gonna do some multithreading)
    usleep(1000000);
    uint64_t elapsed_ticks = mach_absolute_time() - t_start;

    // evaluates cpu usage
    for (size_t i = 0; i < valid; i++) {
        compute_cpu_usage(&p[i], elapsed_ticks);
    }

    // group processes
    process_grouping_pass(p, valid);

    // clean
    free(pid_list);

    ProcessArray *pa = malloc(sizeof(ProcessArray));
    if (!pa) {
        free(p);
        exit(1);
    }

    pa->length = (int) valid;
    pa->p = p;

    return pa;
}
