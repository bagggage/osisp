#pragma once

#include "msg_queue.h"

#include <pthread.h>

void consumer_task(MessageQueue* queue, pthread_cond_t* prod_cond, pthread_cond_t* cons_cond);