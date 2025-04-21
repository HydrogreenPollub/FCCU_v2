#ifndef UART_H
#define UART_H

#define UART_BAUD_RATE 115200
#define UART_PORT_NUM  UART_NUM_0,
#define UART_TXD       40 // set these 4 pins to free/purpose made pins on the pcb
#define UART_RXD       40
#define UART_RTS       40
#define UART_CTS       40

void uart_init();
void uart_send(uint8_t* uart_message);
void uart_receive(uint8_t* uart_message);
void uart_echo();

#endif