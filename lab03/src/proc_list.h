#pragma once

#include <sys/types.h>

typedef struct ProcNode {
    pid_t pid;
    struct ProcNode* next;
} ProcNode;

ProcNode* proc_list_new_node(pid_t pid);
void proc_list_clear(ProcNode** head);

void proc_list_push(ProcNode** head, pid_t pid);
pid_t proc_list_pop(ProcNode** head);

void proc_list_print(const ProcNode* head);