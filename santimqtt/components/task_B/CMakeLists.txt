set(component_srcs "src/taskB.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "include"
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "driver" "color_shared" "mqtt_module"
                       REQUIRES "")
