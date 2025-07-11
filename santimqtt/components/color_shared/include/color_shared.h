#ifndef COLOR_SHARED_H
#define COLOR_SHARED_H

#include <stdint.h>

// Estructura común de color + delay
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint32_t delay_segundos;
} led_color_t;

#endif // COLOR_SHARED_H
