#include "proc_list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

ProcNode* proc_list_new_node(pid_t pid) {
    ProcNode* new_node = (ProcNode*)malloc(sizeof(ProcNode));

    if (new_node == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    new_node->pid = pid;
    new_node->next = NULL;

    return new_node;
}

void proc_list_push(ProcNode** head, pid_t pid) {
    assert(head != NULL);

    if (*head == NULL) {
        *head = proc_list_new_node(pid);
        return;
    }

    ProcNode* cursor = *head;

    while (cursor->next != NULL) cursor = cursor->next;

    cursor->next = proc_list_new_node(pid);
}

void proc_list_print(const ProcNode* head) {
    if (head == NULL) return;

    printf("Parent PID: %d\n", head->pid);

    size_t childIndex = 1;

    while (head->next != NULL) {
        head = head->next;
        printf("Child%lu PID: %d\n", ++childIndex, head->pid);
    }
}

pid_t proc_list_pop(ProcNode** head) {
    assert(head != NULL);

    if (*head == NULL) return -1;

    ProcNode* cursor = *head;
    ProcNode* prev = NULL;

    while (cursor->next != NULL) {
        prev = cursor;
        cursor = cursor->next;
    }

    pid_t pid = cursor->pid;
    free(cursor);

    if (prev == NULL) {
        *head = NULL;
    }
    else {
        prev->next = NULL;
    }

    return pid;
}

void proc_list_clear(ProcNode** head) {
    while (*head != NULL) {
        ProcNode* temp = *head;
        *head = (*head)->next;

        free(temp);
    }
}
