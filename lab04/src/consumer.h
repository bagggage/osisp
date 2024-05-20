#pragma once

#include "msg_queue.h"

void consumer_task(MessageQueue* queue, sem_t* prod_sem, sem_t* cons_sem);