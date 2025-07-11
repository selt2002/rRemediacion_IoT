#include "taskB.h"
#include "color_shared.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "mqtt_module.h"

#define BUF_SIZE 128
#define TAG "TASK_B"
#define MAX_LINE_LENGTH 128

#define ECHO_UART_PORT_NUM (0)
#define ECHO_UART_BAUD_RATE (115200)

static QueueHandle_t taskB_queue = NULL;
static esp_mqtt_client_handle_t s_client = NULL;

static void parse_command(const char *input, led_color_t *cmd)
{
    char color[16];
    int delay;

    if (sscanf(input, "%15s %d", color, &delay) == 2)
    {
        cmd->delay_segundos = delay;

        if (strcasecmp(color, "rojo") == 0)
        {
            cmd->r = 255;
            cmd->g = 0;
            cmd->b = 0;
        }
        else if (strcasecmp(color, "verde") == 0)
        {
            cmd->r = 0;
            cmd->g = 255;
            cmd->b = 0;
        }
        else if (strcasecmp(color, "azul") == 0)
        {
            cmd->r = 0;
            cmd->g = 0;
            cmd->b = 255;
        }
        else
        {
            cmd->r = cmd->g = cmd->b = 0;
            cmd->delay_segundos = 1;
        }
    }
    else
    {
        cmd->r = cmd->g = cmd->b = 0;
        cmd->delay_segundos = 1;
    }
    if (s_client)
    {
        char json[64];
        snprintf(json, sizeof(json),
                 "%s",
                 color);
        esp_mqtt_client_publish(s_client,
                                "/santiagoelters/uart",
                                json, 0, 0, 0);
        // snprintf(json, sizeof(json),
        //          "{\"r\":%d,\"g\":%d,\"b\":%d,\"t\":%lu}",
        //          cmd->r, cmd->g, cmd->b, cmd->delay_segundos);
        // esp_mqtt_client_publish(s_client,
        //                         "/santiagoelters/uart",
        //                         json, 0, 0, 0);
    }
}

static void vTaskB(void *pvParameters)
{
    uint8_t *rx_buffer = malloc(BUF_SIZE);
    char line_buffer[MAX_LINE_LENGTH];
    int line_index = 0;

    while (1)
    {
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, rx_buffer, BUF_SIZE - 1, pdMS_TO_TICKS(100));
        if (len > 0)
        {
            for (int i = 0; i < len; i++)
            {
                char c = (char)rx_buffer[i];

                if (c == '\n' || c == '\r')
                {
                    if (line_index > 0)
                    {

                        line_buffer[line_index] = '\0';
                        ESP_LOGI(TAG, "Recibido: %s", line_buffer);
                        led_color_t cmd;
                        parse_command(line_buffer, &cmd);
                        // char *texto = "textotexto";
                        // xQueueSend(taskB_queue, &texto, portMAX_DELAY);
                        if (xQueueSend(taskB_queue, &cmd, portMAX_DELAY) != pdPASS)
                        {
                            ESP_LOGE(TAG, "No se pudo enviar el comando a la cola");
                        }

                        line_index = 0;
                    }
                }
                else if (line_index < MAX_LINE_LENGTH - 1)
                {
                    line_buffer[line_index++] = c;
                }
                else
                {
                    line_index = 0;
                }
            }
        }
    }

    free(rx_buffer);
    vTaskDelete(NULL);
}

void taskB_uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(ECHO_UART_PORT_NUM, &uart_config);
}

void start_task_b(QueueHandle_t queue)
{
    taskB_queue = queue;
    xTaskCreate(vTaskB, "TaskB", 4096, NULL, 10, NULL);
}

void taskB_set_mqtt_client(esp_mqtt_client_handle_t client)
{
    s_client = client;
}
