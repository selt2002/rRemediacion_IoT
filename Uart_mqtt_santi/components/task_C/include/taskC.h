#ifndef TASK_C_H
#define TASK_C_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "color_shared.h"

void start_task_c(QueueHandle_t queue, SemaphoreHandle_t semaforo, led_color_t *shared_color);

#endif // TASK_C_H
