set(component_srcs "src/mqtt_module.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "include"
                       REQUIRES nvs_flash esp_event esp_netif mqtt esp-tls)
