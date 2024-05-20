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
#include <pthread.h>

#include "consumer.h"
#include "msg_queue.h"
#include "producer.h"

static inline pid_t safe_pthread_create(void* (*entry_point)(void*), void* arg) {
    pthread_t thread;
    pthread_attr_t attributes;

    pthread_attr_init(&attributes);

    int result = pthread_create(&thread, &attributes, entry_point, arg);

    if (result != 0) {
        fprintf(stderr, "Error while creating new thread: %s\nTerminating...\n", strerror(result));
        exit(-3);
    }

    return thread;
}

static inline void safe_sem_init(sem_t* semaphore, unsigned int value) {
    int result = sem_init(semaphore, 0, value);

    if (result != 0) {
        fprintf(stderr, "Error occurred while creating semaphore: %s\n", strerror(result));
        exit(-2);
    }
}

typedef struct Parameters {
    MessageQueue queue;
    sem_t cons_sem;
    sem_t prod_sem;
} Parameters;

static void* start_consumer(void* parameters_ptr) {
    Parameters* parameters = parameters_ptr;
    consumer_task(&parameters->queue, &parameters->prod_sem, &parameters->cons_sem);

    pthread_exit(NULL);
}

static void* start_producer(void* parameters_ptr) {
    Parameters* parameters = parameters_ptr;
    producer_task(&parameters->queue, &parameters->prod_sem, &parameters->cons_sem);

    pthread_exit(NULL);
}

int main() {
    Parameters params;
    params.queue = (MessageQueue) {
        .head = 0,
        .tail = 0,
        .added_count = 0,
        .removed_count = 0
    };

    safe_sem_init(&params.cons_sem, 0);
    safe_sem_init(&params.prod_sem, 0);
    safe_sem_init(&params.queue.lock, 1);

    while (1) {
        char c = getchar();
        getchar();

        if (c == 'q') break;

        switch (c) {
        case 'c': {
            safe_pthread_create(start_consumer, &params);
            break;
        }
        case 'p': {
            safe_pthread_create(start_producer, &params);
            break;
        }
        default: {
            printf("Unknown command: %c\n", c);
            break;
        }
        }
    }

    printf("Exit...\n");

    sem_destroy(&params.cons_sem);
    sem_destroy(&params.prod_sem);
    sem_destroy(&params.queue.lock);

    exit(0);
}