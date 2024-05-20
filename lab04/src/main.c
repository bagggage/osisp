#define __USE_POSIX
#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "consumer.h"
#include "msg_queue.h"
#include "producer.h"
#include "shared.h"

typedef struct Child {
    pid_t* array;
    size_t size;
} Child;

static int shm_id = 0;

Child child = { NULL, 0 };

void add_child(pid_t pid) {
    if (child.size == 0) {
        child.array = (pid_t*)malloc(sizeof(pid_t));

        child.array[0] = pid;
        child.size++;
    }
    else {
        child.size++;
        child.array = (pid_t*)realloc((void*)child.array, child.size * sizeof(pid_t));

        child.array[child.size - 1] = pid;
    }
}

static inline pid_t safe_fork() {
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Error while creating new process: %s\nTerminating...\n", strerror(pid));
        exit(-3);
    }
    if (pid != 0) {
        add_child(pid);
    }

    return pid;
}

static void start_consumer() {
    pid_t pid = safe_fork();

    if (pid == 0) {
        sem_t* prod_sem = sem_open(SEM_PRODUCER, O_RDWR);
        sem_t* cons_sem = sem_open(SEM_CONSUMER, O_RDWR);

        consumer_task((MessageQueue*)shmat(shm_id, NULL, 0), prod_sem, cons_sem);

        sem_close(prod_sem);
        sem_close(cons_sem);
        exit(0);
    }
}

static void start_producer() {
    pid_t pid = safe_fork();

    if (pid == 0) {
        sem_t* prod_sem = sem_open(SEM_PRODUCER, O_RDWR);
        sem_t* cons_sem = sem_open(SEM_CONSUMER, O_RDWR);

        producer_task((MessageQueue*)shmat(shm_id, NULL, 0), prod_sem, cons_sem);

        sem_close(prod_sem);
        sem_close(cons_sem);
        exit(0);
    }
}

int main() {
    sem_unlink(SEM_PRODUCER);
    sem_unlink(SEM_CONSUMER);

    sem_t* cons_sem = sem_open(SEM_CONSUMER, O_CREAT, 0664, 0);
    sem_t* prod_sem = sem_open(SEM_PRODUCER, O_CREAT, 0664, 0);

    shm_id = shmget(0, sizeof(MessageQueue), 0777);

    if (shm_id < 0) {
        fprintf(stderr, "Error while creating shared memory: %s\n", strerror(errno));
        exit(-1);
    }

    MessageQueue* queue = (MessageQueue*)shmat(shm_id, NULL, SHM_RND | SHM_W | SHM_R);
    int result;

    if ((result = sem_init(&queue->lock, 1, 1)) != 0) {
        fprintf(stderr, "Error occurred while creating semaphore: %s\n", strerror(result));
        exit(-2);
    }

    queue->removed_count = 0;
    queue->added_count = 0;
    queue->head = 0;
    queue->tail = 0;

    while (1) {
        char c = getchar();
        getchar();

        if (c == 'q') break;

        switch (c) {
        case 'c': {
            start_consumer();
            break;
        }
        case 'p': {
            start_producer();
            break;
        }
        default: {
            printf("Unknown command: %c\n", c);
            break;
        }
        }
    }

    for (size_t i = 0; i < child.size; ++i) {
        kill(child.array[i], SIGINT);
    }

    sem_close(cons_sem);
    sem_close(prod_sem);

    sem_destroy(&queue->lock);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}