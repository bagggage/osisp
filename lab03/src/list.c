#include "list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

Node* proc_list_new_node(pid_t pid) {
    Node* new_node = (Node*)malloc(sizeof(Node));

    if (new_node == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    new_node->pid = pid;
    new_node->next = NULL;

    return new_node;
}

void proc_list_push(Node** head, pid_t pid) {
    assert(head != NULL);

    if (*head == NULL) {
        *head = proc_list_new_node(pid);
        return;
    }

    Node* cursor = *head;

    while (cursor->next != NULL) cursor = cursor->next;

    cursor->next = proc_list_new_node(pid);
}

void proc_list_print(const Node* head) {
    if (head == NULL) return;

    printf("Parent PID: %d\n", head->pid);

    size_t childIndex = 1;

    while (head->next != NULL) {
        head = head->next;
        printf("Child%lu PID: %d\n", ++childIndex, head->pid);
    }
}

pid_t proc_list_pop(Node** head) {
    assert(head != NULL);

    if (*head == NULL) return -1;

    Node* cursor = *head;
    Node* prev = NULL;

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

void proc_list_clear(Node** head) {
    while (*head != NULL) {
        Node* temp = *head;
        *head = (*head)->next;

        free(temp);
    }
}
