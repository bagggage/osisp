#ifndef LIST_H
#define LIST_H

#include <sys/types.h>

typedef struct Node {
    pid_t pid;
    struct Node* next;
} Node;

Node* proc_list_new_node(pid_t);
void proc_list_push(Node**, pid_t);
void proc_list_print(const Node*);
pid_t proc_list_pop(Node**);
void proc_list_clear(Node**);

#endif //LIST_H
