/*
 * UARTS.h
 *
 *  Created on: 29.07.2013
 *      Author: mrhru
 */

#ifndef UARTS_H_
#define UARTS_H_

#include "lpc17xx_uart.h"

void STD_UART_Init(void);
void _STD_UART_Init(int ch, i32 Baud_rate, UART_PARITY_Type parity);

int std_sendchar(int ch, int c);
void std_msg(int ch, const char* s);
void std_msg_len(int ch, const char* s, u32 len);
u32 _std_sendchar(int ch, u32 c);

void std_set_baud(int ch, i32 baud_rate);

void std_get_clear(int ch);
Bool std_get_available(int ch);
u8 std_get(int ch);


#endif /* UARTS_H_ */
