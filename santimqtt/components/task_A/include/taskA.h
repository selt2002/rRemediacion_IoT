#ifndef TASK_A_H
#define TASK_A_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "color_shared.h"

void start_task_a(SemaphoreHandle_t mutex_color, led_color_t *shared_color);

#endif // TASK_A_H
