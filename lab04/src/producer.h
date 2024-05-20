#pragma once

#include "msg_queue.h"

void producer_task(MessageQueue* queue, sem_t* prod_sem, sem_t* cons_sem);