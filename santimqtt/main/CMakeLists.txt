idf_component_register(
    SRCS "app_main.c" "led_actor.c"
    INCLUDE_DIRS "."
    REQUIRES mqtt_module wifi_manager
)
