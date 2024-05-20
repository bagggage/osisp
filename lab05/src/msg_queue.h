#pragma once

#include <stddef.h>
#include <stdint.h>
#include <semaphore.h>
#include <stdbool.h>

#define MSG_QUEUE_CAPACITY 64
#define MSG_DATA_MAX_SIZE 255

typedef struct Message {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    uint8_t data[MSG_DATA_MAX_SIZE];
} Message;

typedef struct MessageQueue {
    Message messages[MSG_QUEUE_CAPACITY];

    size_t head;
    size_t tail;

    size_t added_count;
    size_t removed_count;

    sem_t lock;
} MessageQueue;

static inline int init_msg_queue(MessageQueue* queue) {
    queue->head = 0;
    queue->tail = 0;
    
    return sem_init(&queue->lock, true, 1);
}

static inline int destroy_msg_queue(MessageQueue* queue) {
    return sem_destroy(&queue->lock);
}
