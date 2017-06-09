/*
 * UART_DEBUG.c
 #include "UART_DEBUG.h"
 *
 *  Created on: 19.06.2010
 *      Author: Eugene Hrulev
 */

#include <string.h>
#include <stdio.h>

#include "types.h"
#include "LPC17xx.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"

#include "uart_driver.h"
#include "UARTS.h"
#include "UART_DEBUG.h"
#include "HAL.h"
#include "common.h"

#define DEBUG_UART_RX_RING_BUFSIZE   128
#define DEBUG_UART_TX_RING_BUFSIZE   4096

// DEBUG UART Ring buffer
//UART_RING_BUFFER_T debug_rb =
//{ 0, // UART Rx ring buffer size
//    0, // UART Rx ring buffer head index
//    0, // UART Rx ring buffer tail index
//    NULL, // UART Rx data ring buffer
//
//    0, // UART Tx ring buffer size
//    0, // UART Tx ring buffer head index
//    0, // UART Tx ring buffer tail index
//    NULL, // UART Tx data ring buffer
//
//    RESET, // Current Tx Interrupt enable state
//    FALSE // UART inited flag
//        };
//
///********************************************************************//**
// * @brief 		UART receive function (ring buffer used)
// * @param[in]	None
// * @return 		None
// *********************************************************************/
//void DEBUG_UART_IntReceive(void)
//{
//	if (!debug_rb.inited) return;
//
//	u8 tmpc;
//	u32 rLen;
//
//	while (1)
//	{
//		// Call UART read function in UART driver
//		rLen = UART_Receive(USART_debug, &tmpc, 1, NONE_BLOCKING);
//		// If data received
//		if (rLen)
//		{
//			/* Check if buffer is more space
//			 * If no more space, remaining character will be trimmed out
//			 */
//			if (!__RX_BUF_IS_FULL(debug_rb))
//			{
//				debug_rb.rx[debug_rb.rx_head] = tmpc;
//
//				debug_rb.rx_head = (debug_rb.rx_head + 1) % debug_rb.rx_buf_size;
//			}
//		}
//		// no more data
//		else
//		{
//			break;
//		}
//	}
//}
//
///********************************************************************//**
// * @brief 		UART transmit function (ring buffer used)
// * @param[in]	None
// * @return 		None
// *********************************************************************/
//void DEBUG_UART_IntTransmit(void)
//{
//	// Disable THRE interrupt
//	UART_IntConfig(USART_debug, UART_INTCFG_THRE, DISABLE);
//	debug_rb.TxIntStat = RESET;
//
//	if (!debug_rb.inited) return;
//
//	/* Wait for FIFO buffer empty, transfer UART_TX_FIFO_SIZE bytes
//	 * of data or break whenever ring buffers are empty */
//	/* Wait until THR empty */
//	while (UART_CheckBusy(USART_debug) == SET)
//		;
//
//	if (!__TX_BUF_IS_EMPTY(debug_rb))
//	{
//		/* Move a piece of data into the transmit FIFO */
//		if (UART_Send(USART_debug, (u8 *) &debug_rb.tx[debug_rb.tx_tail], 1, BLOCKING))
//		{
//			// Update transmit ring FIFO tail pointer
//			debug_rb.tx_tail = (debug_rb.tx_tail + 1) % debug_rb.tx_buf_size;
//		}
//		//		else
//		//		{
//		//			break;
//		//		}
//	}
//
//	// If there is no more data to send, disable the transmit
//	// interrupt - else enable it or keep it enabled
//	if (__TX_BUF_IS_EMPTY(debug_rb))
//	{
//		UART_IntConfig(USART_debug, UART_INTCFG_THRE, DISABLE);
//		// Reset Tx Interrupt state
//		debug_rb.TxIntStat = RESET;
//	}
//	else
//	{
//		// Set Tx Interrupt state
//		debug_rb.TxIntStat = SET;
//		UART_IntConfig(USART_debug, UART_INTCFG_THRE, ENABLE);
//	}
//}
//
///*********************************************************************//**
// * @brief		UART Line Status Error callback
// * @param[in]	bLSErrType	UART Line Status Error Type
// * @return		None
// **********************************************************************/
//void DEBUG_UART_IntErr(u8 bLSErrType)
//{
//}
//
//u32 DEBUG_UART_Receive(u8 *rxbuf, u32 buflen)
//{
//	return (UARTReceive(USART_debug, rxbuf, buflen, &debug_rb));
//}
//
////void DEBUG_UART_Init(i32 Baud_rate)
////{
////	PINSEL_SetPinFunc(DEBUG_RX_Port, DEBUG_RX_bit, DEBUG_RX_FUNC);
////	PINSEL_SetPinFunc(DEBUG_TX_Port, DEBUG_TX_bit, DEBUG_TX_FUNC);
////
////	USART_Init(USART_debug, Baud_rate, DEBUG_UART_IntReceive, DEBUG_UART_IntTransmit, DEBUG_UART_IntErr, &debug_rb,
////	        DEBUG_UART_RX_RING_BUFSIZE, DEBUG_UART_TX_RING_BUFSIZE, "DEBUG");
////}
//
//u32 DEBUG_UART_Send(const char* buf, u32 buflen)
//{
//	return (UARTSend(USART_debug, (u8*) buf, buflen, &debug_rb, &DEBUG_UART_IntTransmit));
//}
//
//void DEBUG_UART_RXBuffer_Clear(void)
//{
//	DI();
//	debug_rb.rx_head = debug_rb.rx_tail = 0;
//	EI();
//}
//
//Bool DEBUG_UART_RXBufferData_Available(void)
//{
//	return (debug_rb.inited && ((debug_rb.rx_head % debug_rb.rx_buf_size) != (debug_rb.rx_tail % debug_rb.rx_buf_size)));
//}
//
//void msg_len(const char* s, u32 len)
//{
//	std_msg_len(3, s, len);
////	DEBUG_UART_Send(s, len);
//}
//
void check_failed(u8 *file, u32 line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	char tmp[120];
	snprintf(tmp, sizeof(tmp), "Wrong parameters value: file %s on line %u", file, line);
	msgn(tmp);
}
//
//u32 _sendchar(u32 c)
//{
//	char buf[2] = " ";
//	buf[0] = c;
//	DEBUG_UART_Send(buf, 1);
//
//	return (c);
//}

//static void printchar(char **str, int c)
//{
//	//extern int putchar(int c);
//
//	if (str) {
//		**str = (char)c;
//		++(*str);
//	}
//	else
//	{
//		(void)putchar(c);
//	}
//}

static int sendchar(int c)
{
	if (c != '\r')
	{
		(void) std_sendchar(USART_debug_channel, c);

		if (c == '\n')
		{
			(void) std_sendchar(USART_debug_channel, '\r');
		}
	}

	return (c);
}

/*
int putchar(int ch)
{
//	if (AbsoluteEnableDebugUart && EnableDebugUart)
	if (AbsoluteEnableDebugUart)
//	if (EnableDebugUart)
	{
		return sendchar(ch);
	}
	else
	{
		return 0;
	}
}
*/
int __sys_write(int iFileHandle, char *pcBuffer, int iLength)
//int __sys_write(int fd, const void* buf, int nbyte)
{
	//char* sbuf = (char*) buf;

	for (u32 i = 0; i < iLength; i ++)
	{

		sendchar(pcBuffer[i]);
//		putchar(sbuf[i]);
	}

	return iLength;
}
