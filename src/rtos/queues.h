#ifndef QUEUES_H
#define QUEUES_H

#include "FreeRTOS.h"
#include "queue.h"

#include "app_config.h"

#define TELEM_QUEUE_LEN 8

QueueHandle_t queues_get_telem(void);

#endif
