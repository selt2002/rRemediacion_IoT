#pragma once
#include "mqtt_client.h"
#ifdef __cplusplus
extern "C" {
#endif
esp_mqtt_client_handle_t mqtt_module_start(esp_event_handler_t event_handler, void *user_ctx);
int mqtt_module_publish(esp_mqtt_client_handle_t client, const char *payload, int qos, int retain);
#ifdef __cplusplus
}
#endif
