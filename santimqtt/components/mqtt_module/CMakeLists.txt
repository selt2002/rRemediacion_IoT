idf_component_register(
    SRCS "mqtt_module.c"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash esp_event esp_netif mqtt esp-tls
)
