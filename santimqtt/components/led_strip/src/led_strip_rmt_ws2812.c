// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include "esp_log.h"
#include "esp_attr.h"
#include "led_strip.h"
#include "driver/rmt.h"

static const char *TAG = "ws2812";
#define STRIP_CHECK(a, str, goto_tag, ret_value, ...)                             \
    do                                                                            \
    {                                                                             \
        if (!(a))                                                                 \
        {                                                                         \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            ret = ret_value;                                                      \
            goto goto_tag;                                                        \
        }                                                                         \
    } while (0)

#define WS2812_T0H_NS (350)
#define WS2812_T0L_NS (1000)
#define WS2812_T1H_NS (1000)
#define WS2812_T1L_NS (350)
#define WS2812_RESET_US (280)

#define KALUGA_RGB_LED_PIN GPIO_NUM_45
#define KALUGA_RGB_LED_NUMBER 1

uint32_t ws2812_t0h_ticks = 0;
uint32_t ws2812_t1h_ticks = 0;
uint32_t ws2812_t0l_ticks = 0;
uint32_t ws2812_t1l_ticks = 0;

led_strip_t *embedded_led;

typedef struct
{
    led_strip_t parent;
    rmt_channel_t rmt_channel;
    uint32_t strip_len;
    uint8_t buffer[0];
} ws2812_t;

/**
 * @brief Conver RGB data to RMT format.
 *
 * @note For WS2812, R,G,B each contains 256 different choices (i.e. uint8_t)
 *
 * @param[in] src: source data, to converted to RMT format
 * @param[in] dest: place where to store the convert result
 * @param[in] src_size: size of source data
 * @param[in] wanted_num: number of RMT items that want to get
 * @param[out] translated_size: number of source data that got converted
 * @param[out] item_num: number of RMT items which are converted from source data
 */
void IRAM_ATTR ws2812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size, size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    if (src == NULL || dest == NULL)
    {
        *translated_size = 0;
        *item_num = 0;
        return;
    }
    const rmt_item32_t bit0 = {{{ws2812_t0h_ticks, 1, ws2812_t0l_ticks, 0}}}; // Logical 0
    const rmt_item32_t bit1 = {{{ws2812_t1h_ticks, 1, ws2812_t1l_ticks, 0}}}; // Logical 1
    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t *pdest = dest;
    while (size < src_size && num < wanted_num)
    {
        for (int i = 0; i < 8; i++)
        {
            // MSB first
            if (*psrc & (1 << (7 - i)))
            {
                pdest->val = bit1.val;
            }
            else
            {
                pdest->val = bit0.val;
            }
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}

esp_err_t ws2812_set_pixel(led_strip_t *strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue)
{
    // ESP_LOGI(TAG, "%s", __func__);

    esp_err_t ret = ESP_OK;
    ws2812_t *ws2812 = __containerof(strip, ws2812_t, parent);
    STRIP_CHECK(index < ws2812->strip_len, "index out of the maximum number of leds", err, ESP_ERR_INVALID_ARG);
    uint32_t start = index * 3;
    // In thr order of GRB
    ws2812->buffer[start + 0] = green & 0xFF;
    ws2812->buffer[start + 1] = red & 0xFF;
    ws2812->buffer[start + 2] = blue & 0xFF;

    ESP_LOGD(TAG, "end of %s", __func__);
    return ESP_OK;
err:
    return ret;
}

esp_err_t ws2812_refresh(led_strip_t *strip, uint32_t timeout_ms)
{
    // ESP_LOGI(TAG, "%s", __func__);

    esp_err_t ret = ESP_OK;
    ws2812_t *ws2812 = __containerof(strip, ws2812_t, parent);
    STRIP_CHECK(rmt_write_sample(ws2812->rmt_channel, ws2812->buffer, ws2812->strip_len * 3, true) == ESP_OK, "transmit RMT samples failed", err, ESP_FAIL);
    ESP_LOGD(TAG, "transmit RMT samples correct");
    ESP_LOGD(TAG, "end of %s", __func__);
    return rmt_wait_tx_done(ws2812->rmt_channel, pdMS_TO_TICKS(timeout_ms));
err:
    return ret;
}

esp_err_t ws2812_clear(led_strip_t *strip, uint32_t timeout_ms)
{
    // ESP_LOGI(TAG, "%s", __func__);

    ws2812_t *ws2812 = __containerof(strip, ws2812_t, parent);
    // Write zero to turn off all leds
    memset(ws2812->buffer, 0, ws2812->strip_len * 3);
    ESP_LOGD(TAG, "end of %s", __func__);
    return ws2812_refresh(strip, timeout_ms);
}

esp_err_t ws2812_del(led_strip_t *strip)
{
    ESP_LOGI(TAG, "%s", __func__);

    ws2812_t *ws2812 = __containerof(strip, ws2812_t, parent);
    free(ws2812);
    return ESP_OK;
}

led_strip_t *led_strip_new_rmt_ws2812(const led_strip_config_t *config)
{
    ESP_LOGI(TAG, "%s", __func__);

    led_strip_t *ret = NULL;
    STRIP_CHECK(config, "configuration can't be null", err, NULL);

    // 24 bits per led
    uint32_t ws2812_size = sizeof(ws2812_t) + config->max_leds * 3;
    ws2812_t *ws2812 = calloc(1, ws2812_size);
    STRIP_CHECK(ws2812, "request memory for ws2812 failed", err, NULL);
    ESP_LOGI(TAG, "request memory for ws2812 correct");

    uint32_t counter_clk_hz = 0;
    STRIP_CHECK(rmt_get_counter_clock((rmt_channel_t)config->dev, &counter_clk_hz) == ESP_OK, "get rmt counter clock failed", err, NULL);
    ESP_LOGD(TAG, "get rmt counter clock correct");
    // ns -> ticks
    float ratio = (float)counter_clk_hz / 1e9;
    ws2812_t0h_ticks = (uint32_t)(ratio * WS2812_T0H_NS);
    ws2812_t0l_ticks = (uint32_t)(ratio * WS2812_T0L_NS);
    ws2812_t1h_ticks = (uint32_t)(ratio * WS2812_T1H_NS);
    ws2812_t1l_ticks = (uint32_t)(ratio * WS2812_T1L_NS);

    // set ws2812 to rmt adapter
    rmt_translator_init((rmt_channel_t)config->dev, ws2812_rmt_adapter);

    ws2812->rmt_channel = (rmt_channel_t)config->dev;
    ws2812->strip_len = config->max_leds;

    ws2812->parent.set_pixel = ws2812_set_pixel;
    ws2812->parent.refresh = ws2812_refresh;
    ws2812->parent.clear = ws2812_clear;
    ws2812->parent.del = ws2812_del;

    return &ws2812->parent;
err:
    return ret;
}

esp_err_t led_rgb_init(led_strip_t **strip)
{
    ESP_LOGI(TAG, "Initializing embedded WS2812");
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(KALUGA_RGB_LED_PIN, RMT_CHANNEL_0);

    /*!< set counter clock to 40MHz */
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(KALUGA_RGB_LED_NUMBER, (led_strip_dev_t)config.channel);
    *strip = led_strip_new_rmt_ws2812(&strip_config);

    if (!(*strip))
    {
        ESP_LOGE(TAG, "install WS2812 driver failed");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "install WS2812 driver correct");

    /*!< Clear LED strip (turn off all LEDs) */
    ESP_ERROR_CHECK((*strip)->clear((*strip), 100));

    return ESP_OK;
}