#include "taskC.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include <stdlib.h>
#include <inttypes.h>
#include "color_shared.h"

static const char *TAG = "TASK_C";

static QueueHandle_t xQueueComandos = NULL;
static SemaphoreHandle_t xSemaforoColor = NULL;
static led_color_t *color_ptr = NULL;

// Callback del timer, actualiza el color compartido
static void timer_callback(TimerHandle_t xTimer)
{
    led_color_t *comando = (led_color_t *)pvTimerGetTimerID(xTimer);

    if (xSemaphoreTake(xSemaforoColor, portMAX_DELAY))
    {
        color_ptr->r = comando->r;
        color_ptr->g = comando->g;
        color_ptr->b = comando->b;
        xSemaphoreGive(xSemaforoColor);

        ESP_LOGI(TAG, "Color actualizado por timer: R=%d G=%d B=%d",
                 comando->r, comando->g, comando->b);
    }

    free(comando);
    xTimerDelete(xTimer, 0);
}

static void vTaskC(void *pvParameters)
{
    ESP_LOGI(TAG, "Tarea C iniciada");

    while (1)
    {
        led_color_t *comando = malloc(sizeof(led_color_t));
        if (!comando)
        {
            ESP_LOGE(TAG, "Fallo malloc comando");
            continue;
        }

        if (xQueueReceive(xQueueComandos, comando, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "Comando recibido: R=%d G=%d B=%d delay=%" PRIu32,
                     comando->r, comando->g, comando->b, comando->delay_segundos);

            TimerHandle_t xTimer = xTimerCreate("CmdTimer",
                                                pdMS_TO_TICKS(comando->delay_segundos * 1000),
                                                pdFALSE,
                                                comando,
                                                timer_callback);

            if (xTimer != NULL)
            {
                if (xTimerStart(xTimer, 0) != pdPASS)
                {
                    ESP_LOGE(TAG, "Fallo al iniciar el timer");
                    free(comando);
                    xTimerDelete(xTimer, 0);
                }
                else
                {
                    ESP_LOGI(TAG, "Timer iniciado (%" PRIu32 " s)", comando->delay_segundos);
                }
            }
            else
            {
                ESP_LOGE(TAG, "Fallo al crear el timer");
                free(comando);
            }
        }
        else
        {
            free(comando);
        }
    }

    vTaskDelete(NULL);
}

void start_task_c(QueueHandle_t queue, SemaphoreHandle_t semaforo, led_color_t *shared_color)
{
    xQueueComandos = queue;
    xSemaforoColor = semaforo;
    color_ptr = shared_color;

    xTaskCreate(vTaskC, "task_c", 4096, NULL, 5, NULL);
}
