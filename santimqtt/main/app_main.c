#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "wifi_manager.h"
#include "mqtt_module.h"
static const char *TAG = "APP";
static void publisher_task(void *pv){
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pv;
    while(1){
        mqtt_module_publish(client, "¡Hola desde ESP32-S2!", 1, 0);
        vTaskDelay(pdMS_TO_TICKS(50000));
    }
}
void app_main(void){
    ESP_LOGI(TAG, "Inicializando Wi‑Fi…");
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_LOGI(TAG, "Arrancando MQTT…");
    esp_mqtt_client_handle_t client = mqtt_module_start(NULL, NULL);
    if(!client){ ESP_LOGE(TAG, "MQTT no pudo arrancar"); return; }
    xTaskCreate(publisher_task, "publisher", 4096, client, 5, NULL);
}
