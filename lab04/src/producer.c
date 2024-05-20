#include "consumer.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>

static sem_t* semaphore;

void fill_random_data(uint8_t* buffer, const uint8_t size) {
    static const char alphabet[] = "ABCDEFGHIGKLMNOPQRSTUVWXYZabcdefghigklmnopqrstuvdxwz";
    const unsigned int alphabet_size = sizeof(alphabet) - 1;

    for (unsigned int i = 0; i < size; ++i) {
        buffer[i] = alphabet[rand() % alphabet_size];
    }

    if (size == MSG_DATA_MAX_SIZE) {
        buffer[MSG_DATA_MAX_SIZE - 1] = '\0';
    }
    else {
        buffer[size] = '\0';
    }
}

uint16_t calculate_hash(const Message* message) {
    const uint8_t* data = (const uint8_t*)message;

    uint16_t result = 0;

    for (unsigned int i = 0; i < (sizeof(Message) - MSG_DATA_MAX_SIZE) + message->size; ++i) {
        result += data[i] << 3;
        result >>= 1;
    }

    return result;
}

static void interrupt_handler() {
    sem_wait(semaphore);
    exit(0);
}

void producer_task(MessageQueue* queue, sem_t* prod_sem, sem_t* cons_sem) {
    semaphore = prod_sem;

    signal(SIGINT, interrupt_handler);

    sem_post(prod_sem);

    srand(time(NULL));

    while(1) {
        unsigned int size = 0;

        while ((size = (rand() % 257)) == 0);

        Message message;
        message.size = size;
        message.hash = 0;
        fill_random_data(message.data, size);
        message.hash = calculate_hash(&message);

        unsigned int new_count = 0;

        bool is_added = false;

        sem_wait(cons_sem);
        sem_post(cons_sem);

        while (is_added == false) {
            sem_wait(&queue->lock);

            if (queue->tail + 1 < MSG_QUEUE_CAPACITY) {
                queue->messages[queue->tail] = message;

                queue->added_count++;
                queue->tail++;

                new_count = queue->added_count;

                is_added = true;
            }

            sem_post(&queue->lock);
        }

        printf("Added message[%u]: size: %u: hash: %x: \"%s\"\n", new_count, message.size, message.hash, message.data);
        sleep(1);
    }

    sem_wait(prod_sem);
}