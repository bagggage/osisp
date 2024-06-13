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

    pthread_detach(thread);

    return thread;
}

typedef struct Parameters {
    MessageQueue queue;
    pthread_cond_t cons_cond;
    pthread_cond_t prod_cond;
} Parameters;

static void* start_consumer(void* parameters_ptr) {
    Parameters* parameters = parameters_ptr;
    consumer_task(&parameters->queue, &parameters->prod_cond, &parameters->cons_cond);

    pthread_exit(NULL);
}

static void* start_producer(void* parameters_ptr) {
    Parameters* parameters = parameters_ptr;
    producer_task(&parameters->queue, &parameters->prod_cond, &parameters->cons_cond);

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

    pthread_cond_init(&params.cons_cond, NULL);
    pthread_cond_init(&params.prod_cond, NULL);
    pthread_mutex_init(&params.queue.lock, NULL);

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

    pthread_mutex_destroy(&params.queue.lock);
    pthread_cond_destroy(&params.cons_cond);
    pthread_cond_destroy(&params.prod_cond);

    exit(0);
}