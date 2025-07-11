set(component_srcs "src/taskC.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "include"
                       PRIV_INCLUDE_DIRS ""
                       PRIV_REQUIRES "led_embebido" "color_shared"
                       REQUIRES "")
