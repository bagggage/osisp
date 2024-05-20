#include "producer.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

static sem_t* semaphore;

static void interrupt_handler() {
    sem_wait(semaphore);
    exit(0);
}

void consumer_task(MessageQueue* queue, sem_t* prod_sem, sem_t* cons_sem) {
    semaphore = cons_sem;

    signal(SIGINT, interrupt_handler);

    sem_post(cons_sem);

    while(1) {
        sem_wait(prod_sem);
        sem_post(prod_sem);

        Message message;
        unsigned int new_count = 0;

        sem_wait(&queue->lock);

        if (queue->added_count > queue->removed_count) {
            message = queue->messages[queue->tail - 1];

            queue->removed_count++;
            queue->tail--;

            new_count = queue->removed_count;
        }
        else {
            sem_post(&queue->lock);
            continue;
        }

        sem_post(&queue->lock);

        printf("Recieved message[%u]: size: %u: hash: %x\n", new_count, message.size, message.hash);
    }
}