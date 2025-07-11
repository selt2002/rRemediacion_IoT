#include "led_embebido.h"
#include "led_strip.h"
#include "esp_log.h"
#include <stdbool.h>

// Usamos funciones de led_strip_rmt_ws2812.c
extern esp_err_t led_rgb_init(led_strip_t **strip);
extern esp_err_t ws2812_set_pixel(led_strip_t *strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue);
extern esp_err_t ws2812_refresh(led_strip_t *strip, uint32_t timeout_ms);
extern esp_err_t ws2812_clear(led_strip_t *strip, uint32_t timeout_ms);

static const char *TAG = "led_embebido";
static led_strip_t *led_strip = NULL;

// Variables de estado del LED
static uint8_t ultimo_r = 0, ultimo_g = 0, ultimo_b = 0;
static bool led_encendido = false;
static uint8_t brillo_actual = 255; // brillo máximo

esp_err_t led_embebido_iniciar(void)
{
    esp_err_t ret = led_rgb_init(&led_strip);
    if (ret != ESP_OK || !led_strip)
    {
        ESP_LOGE(TAG, "Fallo al inicializar el LED RGB");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t led_embebido_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    if (!led_strip)
    {
        ESP_LOGE(TAG, "LED no inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    ultimo_r = red;
    ultimo_g = green;
    ultimo_b = blue;
    led_encendido = true;

    // Se aplica el brillo actual al cambio de color.
    uint8_t r = (red * brillo_actual) / 255;
    uint8_t g = (green * brillo_actual) / 255;
    uint8_t b = (blue * brillo_actual) / 255;

    esp_err_t ret = ws2812_set_pixel(led_strip, 0, r, g, b);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error al establecer el color");
        return ret;
    }

    return ws2812_refresh(led_strip, 100);
}

esp_err_t led_embebido_apagar(void)
{
    if (!led_strip)
    {
        ESP_LOGE(TAG, "LED no inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    return ws2812_clear(led_strip, 100);
}

esp_err_t led_embebido_toggle(void)
{
    if (!led_strip)
    {
        ESP_LOGE(TAG, "LED no inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    if (led_encendido)
    {
        esp_err_t ret = ws2812_clear(led_strip, 100);
        if (ret == ESP_OK)
            led_encendido = false;
        return ret;
    }
    else
    {
        // Si el LED estaba apagado, restaura el último color con brillo actual
        uint8_t r = (ultimo_r * brillo_actual) / 255;
        uint8_t g = (ultimo_g * brillo_actual) / 255;
        uint8_t b = (ultimo_b * brillo_actual) / 255;

        esp_err_t ret = ws2812_set_pixel(led_strip, 0, r, g, b);
        if (ret != ESP_OK)
            return ret;

        ret = ws2812_refresh(led_strip, 100);
        if (ret == ESP_OK)
            led_encendido = true;
        return ret;
    }
}

esp_err_t led_embebido_set_brillo(uint8_t nivel)
{
    if (!led_strip)
    {
        ESP_LOGE(TAG, "LED no inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    if (nivel > 255)
        nivel = 255;
    brillo_actual = nivel;

    if (led_encendido)
    {
        // Mismo color per con nuevo brillo
        uint8_t r = (ultimo_r * brillo_actual) / 255;
        uint8_t g = (ultimo_g * brillo_actual) / 255;
        uint8_t b = (ultimo_b * brillo_actual) / 255;

        esp_err_t ret = ws2812_set_pixel(led_strip, 0, r, g, b);
        if (ret != ESP_OK)
            return ret;

        return ws2812_refresh(led_strip, 100);
    }

    return ESP_OK;
}
