#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>

#include "list.h"

#define COUNT_OF_REPEATS 116
#define SEC_TIMER 16116
#define ARR_SIZE 4
#define SEC_FOR_PREFF_G 5

typedef struct Pair {
    int first;
    int second;
} Pair;

size_t count = 0;
size_t size = 0;

Pair stats[COUNT_OF_REPEATS];
Pair val_stat;

bool is_need_to_contrinue = true;
bool is_need_to_collect = true;
bool flag_p = false;

pid_t parent_pid;

Node* head;

void enable_stat_output();
void disable_stat_output();

void output_stat();
void take_stat();
void make_stat();

// Proc
void create_proc();
void clear_child_proc();
void remove_last_proc();

// Signals
void cmd_to_stat_proc(size_t idx, bool allow_flag, bool query_flag);
void cmd_to_output_all_proc_stat(bool allow_flag);

void allow_all_after_p();

int main() {
    int idx = 0;

    signal(SIGINT, output_stat);
    signal(SIGUSR1, enable_stat_output);
    signal(SIGUSR2, disable_stat_output);

    bool flag_continue = true;

    parent_pid = getpid();
    proc_list_push(&head, parent_pid);

    do {
        int c = getchar();

        switch(c) {
        case '+': create_proc(); break;
        case '-': remove_last_proc(); break;
        case 'l': proc_list_print(head); break;
        case 'k': clear_child_proc(); break;
        case 's': {
            printf("Enter 0 for all processes, or another number for <num> process: ");
            scanf("%d", &idx);

            if (idx == 0) {
                cmd_to_output_all_proc_stat(false);
            }
            else {
                cmd_to_stat_proc(idx, false, false);
            }

            break;
        }
        case 'g': {
            flag_p = false;

            printf("Enter 0 for all processes, or another number for <num> process: ");
            scanf("%d", &idx);

            if (idx == 0) {
                cmd_to_output_all_proc_stat(true);
            }
            else {
                cmd_to_stat_proc(idx, true, false);
            }

            break;
        }
        case 'p': {
            cmd_to_output_all_proc_stat(false);

            printf("Enter the number of the process that will display statistics: ");
            scanf("%d", &idx);

            cmd_to_stat_proc(idx, true, true);

            flag_p = true;

            signal(SIGALRM, allow_all_after_p);
            alarm(SEC_FOR_PREFF_G);

            break;
        }
        case 'q': { clear_child_proc(), flag_continue = false; break; }
        //add wait for zombie_process
        default: flag_continue = false; break;
        }

        getchar();
    } while(flag_continue);

    //free memory from list
    proc_list_clear(&head);

    return 0;
}

void cmd_to_stat_proc(size_t idx, bool allow_flag, bool query_flag) {
    if (count < idx) printf("There is no child process with this number.\n");

    size_t i = 1;
    Node* cursor = head->next;

    while(i++ != idx) cursor = cursor->next;

    if (query_flag) {
        kill(cursor->pid, SIGINT);
        return;
    }

    if (allow_flag) kill(cursor->pid, SIGUSR1);

    kill(cursor->pid, SIGUSR2);
}

void enable_stat_output() {
    is_need_to_collect = true;
}

void disable_stat_output() {
    is_need_to_collect = false;
}

void output_stat() {
    printf("Statistic of child process with PID = %d, PPID = %d All values: ", getpid(), getppid());

    for (size_t i = 0; i < ARR_SIZE; ++i) {
        printf("{%d, %d} ", stats[i].first, stats[i].second);
    }

    printf("\n");
}

void take_stat() {
    stats[size].first = val_stat.first;
    stats[size++].second = val_stat.second;

    is_need_to_contrinue = false;
}

void remove_last_proc() {
    if (head->next == NULL) {
        printf("No child processes.\n");
        return;
    }

    pid_t pid = proc_list_pop(&head);
    kill(pid, SIGKILL);

    printf("Child process with PID = %d successfully deleted.\n", pid);
    printf("Remaining number of child processes: %lu\n", --count);
}

void make_stat() {
    do {
        for (size_t i = 0; i < COUNT_OF_REPEATS; ++i) {
            if (size == ARR_SIZE) size = 0;

            ualarm(SEC_TIMER, 0);

            size_t j = 0;

            do {
                val_stat.first = j % 2;
                val_stat.second = j % 2;
                j++;
            } while (is_need_to_contrinue);

            is_need_to_contrinue = true;
        }

        if (is_need_to_collect) output_stat();
    } while(1);
}

void create_proc() {
    pid_t pid = fork();

    if (pid == 0) {
        signal(SIGALRM, take_stat);
        make_stat();
    }
    else if (pid > 0) {
        proc_list_push(&head, pid);
        count++;
        
        printf("Child process with PID = %d spawned successfully.\n", pid);
    }
}

void cmd_to_output_all_proc_stat(bool allow_flag) {
    if (head->next == NULL) return;

    Node* cursor = head->next;

    while(cursor != NULL) {
        if (allow_flag) {
            kill(cursor->pid, SIGUSR1);
        }
        else {
            kill(cursor->pid, SIGUSR2);
        }

        cursor = cursor->next;
    }
}

void clear_child_proc() {
    while (head->next) {
        pid_t pid = proc_list_pop(&head);
        kill(pid, SIGKILL);

        count--;
    }
    
    printf("All child processes are deleted.\n");
}

void allow_all_after_p() {
    if (flag_p == true) cmd_to_output_all_proc_stat(true);
}
