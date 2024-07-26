#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>

void uart_init();
void uart_send(const char*, uint32_t);

void uart_send_complete();
void uart_received_line(const char*);
#endif // __UART_H__