#include "taskA.h"
#include "freertos/task.h"
#include "led_embebido.h"
#include "esp_log.h"
#include "color_shared.h"

#define TASK_A_DELAY_MS 2000
static const char *TAG = "TASK_A";

static SemaphoreHandle_t mutex = NULL;
static led_color_t *color_ptr = NULL;

static void vTaskA(void *pvParameters)
{
    ESP_LOGI(TAG, "Tarea A iniciada");

    while (1)
    {
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
        {
            led_embebido_set_color(color_ptr->r, color_ptr->g, color_ptr->b);
            xSemaphoreGive(mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_A_DELAY_MS / 2));
        led_embebido_toggle();
        vTaskDelay(pdMS_TO_TICKS(TASK_A_DELAY_MS));
    }

    vTaskDelete(NULL);
}

void start_task_a(SemaphoreHandle_t mutex_color, led_color_t *shared_color)
{
    mutex = mutex_color;
    color_ptr = shared_color;

    xTaskCreate(vTaskA, "task_a", 2048, NULL, 5, NULL);
}
