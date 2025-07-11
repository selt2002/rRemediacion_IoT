idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES mqtt_module wifi_manager driver led_embebido led_strip task_A task_B task_C color_shared
)
