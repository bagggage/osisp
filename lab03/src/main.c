#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "proc_list.h"

#define CMD_BUFFER_SIZE 32
#define COUNT_OF_REPEATS 3
#define SEC_TIMER 1
#define INTERRUPT_DELAY 5

typedef struct Pair {
    int first;
    int second;
} Pair;

typedef struct Stats {
    Pair pairs[256];

    unsigned int size;
    unsigned int ones_count;
    unsigned int one_zero_count;
    unsigned int zero_one_count;
    unsigned int zeroes_count;
} Stats;

bool block_allow_output = false;

Stats stats;
Pair val_stat;

ProcNode* proc_list;
size_t proc_count = 0;

// Child functions
void output_stat();
void child_task();

// Processes operations
void create_proc();
void clear_child_proc();
void remove_last_proc();

// Signal senders
void allow_child_output(size_t idx, bool allow_flag, bool query_flag);
void allow_childs_output(bool allow_flag);

// Signal handlers
void allow_output_handler();
void enable_stat_output();
void disable_stat_output();
void interrupt_handler();
void parent_exit();

void read_command(char* cmd) {
    int i = 0;
    while ((cmd[i] = getchar()) != '\n') {
        i++;

        if (i > 3) {
            cmd[2] = '\0';
            i = 0;
            break;
        }
    }
    cmd[i] = '\0';
}

void kill_childs() {
    ProcNode* curr_proc = proc_list->next;

    while (curr_proc != NULL) {
        kill(curr_proc->pid, SIGKILL);
        curr_proc = curr_proc->next;
    }
}

void parent_exit() {
    kill_childs();
    proc_list_clear(&proc_list);

    exit(0);
}

int main() {
    char command[CMD_BUFFER_SIZE] = { '\0' };

    bool flag_continue = true;

    proc_list_push(&proc_list, getpid());

    printf("Enter operation: ");

    signal(SIGINT, parent_exit);
    signal(SIGALRM, allow_output_handler);
    signal(SIGUSR1, enable_stat_output);
    signal(SIGUSR2, disable_stat_output);

    do {
        read_command(command);

        if (!strcmp(command, "+")) create_proc();
        else if (!strcmp(command, "-")) remove_last_proc();
        else if (!strcmp(command, "l")) proc_list_print(proc_list);
        else if (!strcmp(command, "k")) clear_child_proc();
        else if (!strcmp(command, "s")) allow_childs_output(false);
        else if (!strcmp(command, "g")) allow_childs_output(true);
        else if (!strcmp(command, "q")) break;
        else {
            unsigned int idx = 0;

            if (sscanf(command, "s %u", &idx) == 1) allow_child_output(idx, false, false);
            else if (sscanf(command, "g %u", &idx) == 1) allow_child_output(idx, true, false);
            else if (sscanf(command, "p %u", &idx) == 1) allow_child_output(idx, true, true);
            else puts("Unknown command\n");
        }

    } while(flag_continue);

    parent_exit();
}

void allow_child_output(size_t idx, bool allow_flag, bool query_flag) {
    if (proc_count < idx || idx == 0) {
        printf("There is no child process with this number.\n");
        return;
    }

    size_t i = 1;
    ProcNode* cursor = proc_list->next;

    while(i++ != idx) cursor = cursor->next;

    if (query_flag) {
        block_allow_output = false;

        kill(cursor->pid, SIGINT);
        alarm(INTERRUPT_DELAY);

        return;
    }

    kill(cursor->pid, allow_flag ? SIGUSR1 : SIGUSR2);
}

bool is_can_output = true;

void enable_stat_output() {
    is_can_output = true;
}

void disable_stat_output() {
    is_can_output = false;
}

void output_stat() {
    fprintf(stdout, "Stats of child: PID = %d, PPID = %d, Pairs: {0,0} - %u, {0,1} - %u, {1,0} - %u, {1,1} - %u",
        getpid(), getppid(), stats.zeroes_count, stats.zero_one_count, stats.one_zero_count, stats.ones_count);

    fputc('\n', stdout);
}

void remove_last_proc() {
    if (proc_list->next == NULL) {
        printf("No child processes.\n");
        return;
    }

    pid_t pid = proc_list_pop(&proc_list);
    kill(pid, SIGKILL);

    printf("Removed child: PID = %d\n", pid);
    printf("Number of childs: %lu\n", --proc_count);
}

bool is_need_to_interrupt = false;

void alarm_handler() {
    is_need_to_interrupt = true;

    if (val_stat.first % 2) {
        if (val_stat.second % 2) stats.ones_count++;
        else stats.one_zero_count++;
    }
    else {
        if (val_stat.second % 2) stats.zero_one_count++;
        else stats.zeroes_count++;
    }

    stats.pairs[stats.size].first = val_stat.first;
    stats.pairs[stats.size++].second = val_stat.second;
}

void interrupt_handler() {
    output_stat();
}

void clear_stats() {
    stats.size = 0;
    stats.ones_count = 0;
    stats.one_zero_count = 0;
    stats.zero_one_count = 0;
    stats.zeroes_count = 0;
}

void child_task() {
    clear_stats();

    val_stat.first = 0;
    val_stat.second = 0;

    unsigned int i = 0;

    while (1) {
        if (i == COUNT_OF_REPEATS) {
            i = 0;

            if (is_can_output) output_stat();

            clear_stats();
        }

        alarm(SEC_TIMER);

        while (!is_need_to_interrupt) {
            if (val_stat.first) {
                if (val_stat.second) { val_stat = (Pair){ 0, 0 }; }
                else { val_stat = (Pair){ 1, 1 }; }
            }
            else {
                if (val_stat.second) { val_stat = (Pair){ 1, 0 }; }
                else { val_stat = (Pair){ 0, 1 }; }
            }
        }

        is_need_to_interrupt = false;

        i++;
    }

    exit(0);
}

void create_proc() {
    pid_t pid = fork();

    if (pid == 0) {
        signal(SIGALRM, alarm_handler);
        signal(SIGINT, interrupt_handler);

        child_task();
    }
    else if (pid > 0) {
        proc_list_push(&proc_list, pid);
        proc_count++;

        printf("Added child: PID = %d\n", pid);
    }
}

void allow_childs_output(bool allow_flag) {
    block_allow_output = !allow_flag;

    if (proc_list->next == NULL) return;

    ProcNode* curr_proc = proc_list->next;

    const unsigned int signal = allow_flag ? SIGUSR1 : SIGUSR2;

    while(curr_proc != NULL) {
        kill(curr_proc->pid, signal);

        curr_proc = curr_proc->next;
    }
}

void clear_child_proc() {
    while (proc_list->next) {
        pid_t pid = proc_list_pop(&proc_list);
        kill(pid, SIGKILL);

        proc_count--;
    }
    
    printf("All child processes are deleted.\n");
}

void allow_output_handler() {
    if (block_allow_output == false) allow_childs_output(true);
}
