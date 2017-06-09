/*
 * uart_driver.h
 *
 *  Created on: 19.06.2010
 *      Author: Eugene Hrulev
 */

#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_

/* Check buf is full or not */
#define __RX_BUF_IS_FULL(rb) ((rb.rx_tail % rb.rx_buf_size) == ((rb.rx_head + 1) % rb.rx_buf_size))
/* Check buf is empty */
#define __TX_BUF_IS_EMPTY(rb) ((rb.tx_head % rb.tx_buf_size) == (rb.tx_tail % rb.tx_buf_size))

/** @brief UART Ring buffer structure */
typedef struct
{
		__IO
		u32 rx_buf_size; // UART Rx ring buffer size
//	__IO u32 rx_buf_mask; // UART Rx ring buffer size
		__IO
		u32 rx_head; // UART Rx ring buffer head index
		__IO
		u32 rx_tail; // UART Rx ring buffer tail index
		__IO
		u8* rx; // UART Rx data ring buffer

		__IO
		u32 tx_buf_size; // UART Tx ring buffer size
//	__IO u32 tx_buf_mask; // UART Tx ring buffer size
		__IO
		u32 tx_head; // UART Tx ring buffer head index
		__IO
		u32 tx_tail; // UART Tx ring buffer tail index
		__IO
		u8* tx; // UART Tx data ring buffer

		__IO
		FlagStatus TxIntStat; // Current Tx Interrupt enable state
		__IO
		Bool inited; // UART inited flag
} UART_RING_BUFFER_T;

void USART_Init(LPC_UART_TypeDef *UARTx, i32 Baud_rate, UART_PARITY_Type parity, void* IntReceive_Proc, void* IntTransmit_Proc, void* IntErr_Proc, UART_RING_BUFFER_T* ring_buf,
                u32 rx_buf_size, u32 tx_buf_size, const char* UART_name);

void USART_SetBaud(LPC_UART_TypeDef *UARTx, i32 Baud_rate);

u32 UARTSend(LPC_UART_TypeDef *UARTPort, u8 *buf, u32 buflen, UART_RING_BUFFER_T* ring_buf, fnRxCbs_Type* UART_IntTransmit);

u32 UARTReceive(LPC_UART_TypeDef *UARTPort, u8 *buf, u32 buflen, UART_RING_BUFFER_T* ring_buf);

void Uart_Ring_Buffer_Init(UART_RING_BUFFER_T* rb, u32 rx_buf_size, u32 tx_buf_size);

#endif /* UART_DRIVER_H_ */

