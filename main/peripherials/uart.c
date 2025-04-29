#include "uart.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <stdlib.h>

#define UART_PORT_NUM    UART_NUM_1
#define UART_BAUD_RATE   115200
#define UART_TX_PIN      17
#define UART_RX_PIN      18
#define UART_BUFFER_SIZE 1024

static const char* TAG = "UART";

void uart_init(void)
{
    uart_config_t uart_config = { .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT };
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUFFER_SIZE * 2, UART_BUFFER_SIZE * 2, 0, NULL, 0));
    ESP_LOGI(TAG, "UART%d initialized at %d baud", UART_PORT_NUM, UART_BAUD_RATE);
}

int uart_send_data(const char* data, size_t length)
{
    int bytes = uart_write_bytes(UART_PORT_NUM, data, length);
    ESP_LOGI(TAG, "uart_send_data: sent %d bytes", bytes);
    return bytes;
}

int uart_receive_data(uint8_t* buffer, size_t length, TickType_t timeout)
{
    int bytes = uart_read_bytes(UART_PORT_NUM, buffer, length, timeout);
    if (bytes > 0)
    {
        ESP_LOGI(TAG, "uart_receive_data: received %d bytes", bytes);
    }
    printf("%s \n", buffer);
    return bytes;
}

void uart_echo_task(void* arg)
{
    uint8_t* data = malloc(UART_BUFFER_SIZE);
    if (!data)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for echo buffer");
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        int len = uart_receive_data(data, UART_BUFFER_SIZE, pdMS_TO_TICKS(1000));
        if (len > 0)
        {
            uart_send_data((const char*) data, len);
        }
    }

    free(data);
    vTaskDelete(NULL);
}