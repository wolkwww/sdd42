/**
 * @file	: uart_interrupt_test.c
 * @purpose	: An example of UART using interrupt mode to test the UART driver
 * @version	: 1.0
 * @date	: 18. Mar. 2009
 * @author	: HieuNguyen
 *----------------------------------------------------------------------------
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#include "types.h"

#include <stdlib.h>
#include <stdio.h>

#include "lpc_types.h"
#include "lpc17xx_uart.h"		/* Central include file */
#include "lpc17xx_libcfg.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_pinsel.h"

#include "uart_driver.h"
#include "my_mem.h"
#include "HAL.h"
#include "UART_DEBUG.h"

/************************** PRIVATE MACROS *************************/

///* Buf mask */
//#define __BUF_MASK (UART_RING_BUFSIZE-1)
///* Check buf is full or not */
//#define __BUF_IS_FULL(head, tail) ((tail&__BUF_MASK)==((head+1)&__BUF_MASK))
///* Check buf will be full in next receiving or not */
//#define __BUF_WILL_FULL(head, tail) ((tail&__BUF_MASK)==((head+2)&__BUF_MASK))
///* Check buf is empty */
//#define __BUF_IS_EMPTY(head, tail) ((head&__BUF_MASK)==(tail&__BUF_MASK))
///* Reset buf */
//#define __BUF_RESET(bufidx)	(bufidx=0)
//#define __BUF_INCR(bufidx)	(bufidx=(bufidx+1)&__BUF_MASK)
/************************** PRIVATE VARIABLES *************************/
//u8 menu1[] = "Hello NXP Semiconductors \n";
//u8 menu2[] =
//"UART interrupt mode demo using ring buffer \n\t "
//"MCU LPC17xx - ARM Cortex-M3 \n\t "
//"UART0 - 9600bps \n";
//u8 menu3[] = "UART demo terminated!\n";
//
//// UART Ring buffer
//UART_RING_BUFFER_T rb;
//
//// Current Tx Interrupt enable state
//__IO FlagStatus TxIntStat;
/************************** PRIVATE FUNCTIONS *************************/
void UART0_IRQHandler(void);
void UART1_IRQHandler(void);
void UART2_IRQHandler(void);
void UART3_IRQHandler(void);

//void UART_IntTransmit(void);
//void UART_IntReceive(void);

void UART0_IRQHandler(void)
{
	// Call Standard UART 1 interrupt handler
	UART0_StdIntHandler();
}

/*********************************************************************//**
 * @brief	UART1 interrupt handler sub-routine reference, just to call the
 * 				standard interrupt handler in uart driver
 * @param	None
 * @return	None
 **********************************************************************/
void UART1_IRQHandler(void)
{
	// Call Standard UART 1 interrupt handler
	UART1_StdIntHandler();
}

/*********************************************************************//**
 * @brief	UART2 interrupt handler sub-routine reference, just to call the
 * 				standard interrupt handler in uart driver
 * @param	None
 * @return	None
 **********************************************************************/
void UART2_IRQHandler(void)
{
	// Call Standard UART 2 interrupt handler
	UART2_StdIntHandler();
}

/*********************************************************************//**
 * @brief	UART3 interrupt handler sub-routine reference, just to call the
 * 				standard interrupt handler in uart driver
 * @param	None
 * @return	None
 **********************************************************************/
void UART3_IRQHandler(void)
{
	// Call Standard UART 3 interrupt handler
	UART3_StdIntHandler();
}

/*********************************************************************//**
 * @brief		UART transmit function for interrupt mode (using ring buffers)
 * @param[in]	UARTPort	Selected UART peripheral used to send data,
 * 				should be UART0
 * @param[out]	txbuf Pointer to Transmit buffer
 * @param[in]	buflen Length of Transmit buffer
 * @return 		Number of bytes actually sent to the ring buffer
 **********************************************************************/
u32 UARTSend(LPC_UART_TypeDef *UARTPort, u8 *buf, u32 buflen, UART_RING_BUFFER_T* ring_buf, fnTxCbs_Type* UART_IntTransmit)
{
	if (!ring_buf->inited)
		return (0);

	u8 *data = (u8 *) buf;
	u32 bytes = 0;

	/* Temporarily lock out UART transmit interrupts during this
	 read so the UART transmit interrupt won't cause problems
	 with the index values */
	UART_IntConfig(UARTPort, UART_INTCFG_THRE, DISABLE);

	// Loop until transmit run buffer is full or until n_bytes expires
	while ((buflen != 0) && ((ring_buf->tx_tail % ring_buf->tx_buf_size) != ((ring_buf->tx_head + 1) % ring_buf->tx_buf_size)))
	{
		// Write data from buffer into ring buffer
		ring_buf->tx[ring_buf->tx_head] = *data;
		data++;

		// Increment head pointer
		//		__BUF_INCR(ring_buf->tx_head);
		ring_buf->tx_head = (ring_buf->tx_head + 1) % ring_buf->tx_buf_size;

		// Increment data count and decrement buffer size count
		bytes++;
		buflen--;
	}

	//	Check if current Tx interrupt enable is reset,
	//	that means the Tx interrupt must be re-enabled
	//	due to call UART_IntTransmit() function to trigger
	//	this interrupt type
	if (ring_buf->TxIntStat == RESET)
	{
		UART_IntTransmit();
	}
	//Otherwise, re-enables Tx Interrupt
	else
	{
		UART_IntConfig(UARTPort, UART_INTCFG_THRE, ENABLE);
	}

	return (bytes);
}

/*********************************************************************//**
 * @brief		UART read function for interrupt mode (using ring buffers)
 * @param[in]	UARTPort	Selected UART peripheral used to send data,
 * 				should be UART0
 * @param[out]	rxbuf Pointer to Received buffer
 * @param[in]	buflen Length of Received buffer
 * @return 		Number of bytes actually read from the ring buffer
 **********************************************************************/
u32 UARTReceive(LPC_UART_TypeDef *UARTPort, u8 *buf, u32 buflen, UART_RING_BUFFER_T* ring_buf)
{
	if (!ring_buf->inited)
		return (0);

	u8 *data = (u8 *) buf;
	u32 bytes = 0;

	/* Temporarily lock out UART receive interrupts during this
	 read so the UART receive interrupt won't cause problems
	 with the index values */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, DISABLE);

	/* Loop until receive buffer ring is empty or
	 until max_bytes expires */
	while ((buflen != 0) && (!((ring_buf->rx_head % ring_buf->rx_buf_size) == (ring_buf->rx_tail % ring_buf->rx_buf_size))))
	{
		/* Read data from ring buffer into user buffer */
		*data = ring_buf->rx[ring_buf->rx_tail];
		data++;

		/* Update tail pointer */
		//		__BUF_INCR(rb.rx_tail);
		ring_buf->rx_tail = (ring_buf->rx_tail + 1) % ring_buf->rx_buf_size;

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/* Re-enable UART interrupts */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, ENABLE);

	return (bytes);
}

void Uart_Ring_Buffer_Init(UART_RING_BUFFER_T* rb, u32 rx_buf_size, u32 tx_buf_size)
{
	rb->inited = FALSE;

	//RX
	if (rb->rx_buf_size == 0)
	{
		rb->rx_buf_size = rx_buf_size;
		rb->rx = (u8*) fix_malloc(rb->rx_buf_size, "UART RX");
	}
	rb->rx_head = 0;
	rb->rx_tail = 0;

	//TX
	if (rb->tx_buf_size == 0)
	{
		rb->tx_buf_size = tx_buf_size;
		rb->tx = (u8*) fix_malloc(rb->tx_buf_size, "UART TX");
	}
	rb->tx_head = 0;
	rb->tx_tail = 0;

	rb->TxIntStat = RESET;
}

void ShowUartInfo(const char* UART_name, UART_RING_BUFFER_T* ring_buf, i32 Baud_rate);

void USART_SetBaud(LPC_UART_TypeDef *UARTx, i32 Baud_rate)
{
	UART_CFG_Type UARTConfigStruct;
	UART_ConfigStructInit(&UARTConfigStruct);

	// Re-configure baudrate to speed
	UARTConfigStruct.Baud_rate = Baud_rate;

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init(UARTx, &UARTConfigStruct);
}

void USART_Init(LPC_UART_TypeDef *UARTx, i32 Baud_rate, UART_PARITY_Type parity, void* IntReceive_Proc, void* IntTransmit_Proc, void* IntErr_Proc, UART_RING_BUFFER_T* ring_buf, u32 rx_buf_size,
    u32 tx_buf_size, const char* UART_name)
{
	// Reset ring buf head and tail idx
	Uart_Ring_Buffer_Init(ring_buf, rx_buf_size, tx_buf_size);

	UART_CFG_Type UARTConfigStruct;
	UART_ConfigStructInit(&UARTConfigStruct);

	// Re-configure baudrate to speed
	UARTConfigStruct.Baud_rate = Baud_rate;
	UARTConfigStruct.Parity = parity;

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init(UARTx, &UARTConfigStruct);

	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	// Initialize FIFOConfigStruct to default state:
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(UARTx, &UARTFIFOConfigStruct);

	// Setup callback ---------------
	// Receive callback
	UART_SetupCbs(UARTx, 0, (void *) IntReceive_Proc);
	// Transmit callback
	UART_SetupCbs(UARTx, 1, (void *) IntTransmit_Proc);
	// Line Status Error callback
	UART_SetupCbs(UARTx, 3, (void *) IntErr_Proc);

	// Enable UART Transmit
	UART_TxCmd(UARTx, ENABLE);

	/* Enable UART Rx interrupt */
	UART_IntConfig(UARTx, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig(UARTx, UART_INTCFG_RLS, ENABLE);

	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */

	i32 UARTx_IRQn = -1;

	if ((u32) UARTx == (u32) LPC_UART0)
		UARTx_IRQn = UART0_IRQn;
	else if ((u32) UARTx == (u32) LPC_UART1)
		UARTx_IRQn = UART1_IRQn;
	else if ((u32) UARTx == (u32) LPC_UART2)
		UARTx_IRQn = UART2_IRQn;
	else if ((u32) UARTx == (u32) LPC_UART3)
		UARTx_IRQn = UART3_IRQn;

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UARTx_IRQn, ((0x01 << 3) | 0x01));
	/* Enable Interrupt for UART2 channel */
	NVIC_EnableIRQ(UARTx_IRQn);
	ring_buf->inited = TRUE;

	if (Debug_UART)
	{
		ShowUartInfo(UART_name, ring_buf, Baud_rate);
	}
}

void ShowUartInfo(const char* UART_name, UART_RING_BUFFER_T* ring_buf, i32 Baud_rate)
{
	printf("%s - %u BAUD\n\tRX BUFF= 0x%X ... 0x%X (%u)\n\tTX BUFF= 0x%X ... 0x%X (%u)", UART_name, Baud_rate, (u32) (ring_buf->rx), (u32) (ring_buf->rx) + ring_buf->rx_buf_size,
	ring_buf->rx_buf_size, (u32) (ring_buf->tx), (u32) (ring_buf->tx) + ring_buf->tx_buf_size, ring_buf->tx_buf_size);
	delay(100);
}
