/*
 #include "UART_DEBUG.h"
 * UART_DEBUG.h
 *
 *  Created on: 19.06.2010
 *      Author: Eugene Hrulev
 */

#ifndef UART_DEBUG_H_
#define UART_DEBUG_H_

#include <stdint.h>
#include "types.h"

//#define USART_debug		LPC_UART3

#define USART_debug		LPC_UART0
#define USART_debug_channel		0

//int putchar(int c);

//char putc (char c);

void DEBUG_UART_Init(i32 speed);
u32 DEBUG_UART_Receive(u8 *rxbuf, u32 buflen);
u32 DEBUG_UART_Send(const char* buf, u32 buflen);

void DEBUG_UART_RXBuffer_Clear(void);
bool DEBUG_UART_RXBufferData_Available(void);


void msg_len(const char* s, u32 len);
int _write(int fd, const void *buf, u32 nbyte);

#endif /* UART_DEBUG_H_ */
