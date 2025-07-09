#ifndef TASK_B_H
#define TASK_B_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "color_shared.h"

// Inicialización del UART
void taskB_uart_init(void);

// Inicialización de la TaskB
void start_task_b(QueueHandle_t queue);

// estructura de mi mensaje mqtt

#endif // TASK_B_H
