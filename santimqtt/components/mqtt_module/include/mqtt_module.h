
#include "mqtt_client.h"

esp_mqtt_client_handle_t mqtt_module_start(esp_event_handler_t event_handler, void *user_ctx);
int mqtt_module_publish(esp_mqtt_client_handle_t client, const char *payload, int qos, int retain);
