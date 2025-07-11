#ifndef TASK_B_H
#define TASK_B_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "color_shared.h"
#include "mqtt_client.h"

// Inicialización del UART
void taskB_uart_init(void);

// Inicialización de la TaskB
void start_task_b(QueueHandle_t queue);

// Setea el mqtt
void taskB_set_mqtt_client(esp_mqtt_client_handle_t client);

#endif // TASK_B_H
