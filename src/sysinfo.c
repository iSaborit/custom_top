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

    proc_set_pid(p, pid);
    proc_set_collapsed(p, 0);
    proc_set_title_from_pid(p, pid);
    proc_set_ram(p, (int)(task_info.pti_resident_size / 1024 / 1024));
    proc_set_cpu_snapshot(p, task_info.pti_total_user + task_info.pti_total_system);

    struct proc_bsdinfo bsd;
    ret = proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &bsd, sizeof(bsd));

    if (ret == sizeof(bsd)) {
        proc_set_state(p, convert_run_state((int) bsd.pbi_status));

        const struct passwd *pwd = getpwuid(bsd.pbi_uid);
        if (pwd) {
            proc_set_user(p, pwd->pw_name);
        }

        proc_set_gid(p, (int) bsd.pbi_gid);
        proc_set_ppid(p, (int) bsd.pbi_ppid);

        if (bsd.pbi_pid == bsd.pbi_gid) {
            proc_set_is_parent(p, 1);
        }
    }

    return 0;
}

static void compute_cpu_usage(Process *p, uint64_t elapsed_ticks) {
    struct proc_taskinfo task_info;
    int ret = proc_pidinfo(proc_get_pid(p), PROC_PIDTASKINFO, 0, &task_info, sizeof(task_info));

    if (ret != sizeof(task_info)) {
        proc_set_cpu(p, 0.0f);
        return;
    }

    uint64_t current = task_info.pti_total_user + task_info.pti_total_system;
    uint64_t delta = current - proc_get_cpu_snapshot(p);

    proc_set_cpu(p, (float) delta / (float) elapsed_ticks * 100.0f);
}


/*
 * Collapsing process
 */

static void add_process_data(Process *parent, Process *child) {
    proc_add_cpu(parent, proc_get_cpu(child));
    proc_add_ram(parent, proc_get_ram(child));
}

static void collapse_on_parent(ProcessArray *pa, Process *p) {
    int length = proc_array_length(pa);
    for (int i = 0; i < length; i++) {
        Process *candidate = proc_array_get(pa, i);
        if (proc_get_pid(candidate) == proc_get_ppid(p)) {
            add_process_data(candidate, p);
            proc_set_collapsed(p, 1);
            break;
        }
    }
}

static void process_grouping_pass(ProcessArray *pa) {
    int length = proc_array_length(pa);
    for (int i = 0; i < length; i++) {
        Process *p = proc_array_get(pa, i);
        // Only group if the parent is NOT launchd (1) or kernel_task (0)
        if (!proc_get_is_parent(p) && proc_get_ppid(p) > 1) {
            collapse_on_parent(pa, p);
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
    ProcessArray *pa = proc_array_create(number_of_pids);
    if (!pa) {
        free(pid_list);
        exit(1);
    }

    // static info + cpu snapshot
    int valid = 0;
    uint64_t t_start = mach_absolute_time();

    for (size_t i = 0; i < number_of_pids; i++) {
        if (fill_process_static(proc_array_get(pa, valid), pid_list[i]) == 0) {
            valid++;
        }
    }

    proc_array_set_length(pa, valid);

    // should refactor this bc blocks the ui (gonna do some multithreading)
    usleep(1000000);
    uint64_t elapsed_ticks = mach_absolute_time() - t_start;

    // evaluates cpu usage
    for (int i = 0; i < valid; i++) {
        compute_cpu_usage(proc_array_get(pa, i), elapsed_ticks);
    }

    // group processes
    process_grouping_pass(pa);

    // clean
    free(pid_list);

    return pa;
}
