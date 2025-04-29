#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Initialize UART peripheral (baud rate, pins, driver install).
 */
void uart_init(void);

/**
 * @brief Send raw data out over UART.
 *
 * @param data    Pointer to buffer to send.
 * @param length  Number of bytes to send.
 * @return Number of bytes actually written.
 */
int uart_send_data(const char* data, size_t length);

/**
 * @brief Receive raw data from UART.
 *
 * @param buffer   Buffer to fill with incoming data.
 * @param length   Maximum bytes to read.
 * @param timeout  FreeRTOS ticks to wait (e.g. pdMS_TO_TICKS(1000)).
 * @return Number of bytes actually read.
 */
int uart_receive_data(uint8_t* buffer, size_t length, TickType_t timeout);

/**
 * @brief FreeRTOS task that continually echoes received bytes back.
 *
 * @param arg  Unused; can be NULL.
 */
void uart_echo_task(void* arg);

#endif // UART_H