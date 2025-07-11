#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "wifi_manager.h"
#include "mqtt_module.h"
#include "freertos/FreeRTOS.h"
#include "taskA.h"
#include "taskB.h"
#include "taskC.h"
#include "led_embebido.h"
#include "led_strip.h"

led_color_t color_global = {255, 0, 0, 1};
SemaphoreHandle_t semaforo_color;
QueueHandle_t xQueueComandos;
static const char *TAG = "APP";

static void publisher_task(void *pv)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pv;
    while (1)
    {
        mqtt_module_publish(client, "keep-alive de santi", 1, 0);
        vTaskDelay(pdMS_TO_TICKS(50000));
    }
}
void app_main(void)
{
    ESP_LOGI(TAG, "Inicializando Wi‑Fi…");
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_LOGI(TAG, "Arrancando MQTT…");
    esp_mqtt_client_handle_t client = mqtt_module_start(NULL, NULL);
    if (!client)
    {
        ESP_LOGE(TAG, "MQTT no pudo arrancar");
        return;
    }

    xTaskCreate(publisher_task, "publisher", 4096, client, 5, NULL);

    led_embebido_iniciar();

    // Inicializo UART
    taskB_uart_init();

    // Semáforo
    semaforo_color = xSemaphoreCreateMutex();
    if (semaforo_color == NULL)
    {
        printf("Error al crear el mutex\n");
        return;
    }

    // Cola de comandos (tiene elementos led_color_t)
    xQueueComandos = xQueueCreate(10, sizeof(led_color_t));

    if (xQueueComandos == NULL)
    {
        printf("Error al crear la queue\n");
        return;
    }

    // Inicio de tareas
    taskB_set_mqtt_client(client);
    start_task_b(xQueueComandos);                                // Recibe comandos por UART y los encola
    start_task_c(xQueueComandos, semaforo_color, &color_global); // Toma comandos de la cola y actualiza el color compartido
    start_task_a(semaforo_color, &color_global);                 // LED parpadea con color elegido
}
