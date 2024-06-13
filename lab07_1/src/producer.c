#include "producer.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

static pthread_mutex_t* mutex;

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
    pthread_mutex_lock(mutex);
    exit(0);
}

void producer_task(MessageQueue* queue, pthread_cond_t* prod_cond, pthread_cond_t* cons_cond) {
    mutex = &queue->lock;

    signal(SIGINT, interrupt_handler);
    srand(time(NULL));

    while(1) {
        // Generate
        unsigned int size = 0;

        while ((size = (rand() % 257)) == 0);

        Message message;
        message.size = size;
        message.hash = 0;
        fill_random_data(message.data, size);
        message.hash = calculate_hash(&message);

        unsigned int new_count = 0;
        //

        pthread_mutex_lock(mutex);

        if (queue->tail + 1 >= MSG_QUEUE_CAPACITY) pthread_cond_wait(cons_cond, mutex);

        // Add
        queue->messages[queue->tail] = message;

        queue->added_count++;
        queue->tail++;

        new_count = queue->added_count;
        //

        pthread_mutex_unlock(mutex);
        pthread_cond_signal(prod_cond);

        printf("Added message[%u]: size: %u: hash: %x: \"%s\"\n", new_count, message.size, message.hash, message.data);
        sleep(1);
    }
}