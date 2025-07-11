#include "sdkconfig.h"
#include "mqtt_module.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
static const char *TAG = "mqtt_module";
static const char *topic_base = CONFIG_MQTT_MODULE_TOPIC_BASE; //"/santiagoelters/uart";
static void default_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t ev = event_data;
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Conectado al broker");
        esp_mqtt_client_subscribe(ev->client, topic_base, 1);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Mensaje: %.*s -> %.*s", ev->topic_len, ev->topic, ev->data_len, ev->data);
        break;
    default:
        break;
    }
}
esp_mqtt_client_handle_t mqtt_module_start(esp_event_handler_t cb, void *ctx)
{
    static bool nvs_ok = false;
    if (!nvs_ok)
    {
        ESP_ERROR_CHECK(nvs_flash_init());
        nvs_ok = true;
    }
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = CONFIG_MQTT_MODULE_BROKER_URI, //"mqtt://broker.hivemq.com",
        .network.timeout_ms = 2000,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&cfg);
    if (!client)
    {
        ESP_LOGE(TAG, "Fallo al crear cliente MQTT");
        return NULL;
    }
    esp_event_handler_t h = cb ? cb : default_event_handler;
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, h, ctx));
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
    return client;
}
int mqtt_module_publish(esp_mqtt_client_handle_t client, const char *payload, int qos, int retain)
{
    if (!client || !payload)
        return -1;
    return esp_mqtt_client_publish(client, topic_base, payload, 0, qos, retain);
}
