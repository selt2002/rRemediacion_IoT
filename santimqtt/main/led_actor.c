#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#define LED_GPIO 18
void led_init(void){
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL<<LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&cfg);
}
void led_on(void){ gpio_set_level(LED_GPIO,1); }
void led_off(void){ gpio_set_level(LED_GPIO,0); }
