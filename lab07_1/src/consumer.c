#include "consumer.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

static pthread_mutex_t* mutex;

static void interrupt_handler() {
    pthread_mutex_lock(mutex);
    exit(0);
}

void consumer_task(MessageQueue* queue, pthread_cond_t* prod_cond, pthread_cond_t* cons_cond) {
    mutex = &queue->lock;

    signal(SIGINT, interrupt_handler);

    while (1) {
        pthread_mutex_lock(mutex);
        if (queue->added_count <= queue->removed_count) pthread_cond_wait(prod_cond, mutex);

        Message message;
        unsigned int new_count = 0;

        message = queue->messages[queue->tail - 1];

        queue->removed_count++;
        queue->tail--;

        new_count = queue->removed_count;

        pthread_mutex_unlock(mutex);
        pthread_cond_signal(cons_cond);
        printf("Recieved message[%u]: size: %u: hash: %x\n", new_count, message.size, message.hash);
    }
}