set(component_srcs "main.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "."
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver" "task_B" "task_C" "color_shared" "nvs_flash" "esp_event" "esp_netif" "mqtt"
                       REQUIRES "")
