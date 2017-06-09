/*
 * UARTS.c
 *
 *  Created on: 29.07.2013
 *      Author: mrhru
 */

#include <string.h>
#include <stdio.h>

#include "types.h"
#include "LPC17xx.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"

#include "uart_driver.h"
#include "UARTS.h"
#include "HAL.h"
#include "common.h"

#define UART_COUNT					4

#define STD_UART_RX_RING_BUFSIZE	128
#define STD_UART_TX_RING_BUFSIZE	128

#define DEBUG_UART_RX_RING_BUFSIZE   128
//#define DEBUG_UART_TX_RING_BUFSIZE   4096
#define DEBUG_UART_TX_RING_BUFSIZE   2048

typedef struct
{
	LPC_UART_TypeDef* LPC_UART;
	UART_RING_BUFFER_T rb;

	fnRxCbs_Type* RxFunc;
	fnTxCbs_Type* TxFunc;

	u8 rx_port;
	u8 rx_bit;
	u8 rx_func;

	u8 tx_port;
	u8 tx_bit;
	u8 tx_func;

	u32 rx_buf_size;
	u32 tx_buf_size;

	const char* name;

} UART_DEF;

void UART_0_IntReceive(void);
void UART_1_IntReceive(void);
void UART_2_IntReceive(void);
void UART_3_IntReceive(void);

void UART_0_IntTransmit(void);
void UART_1_IntTransmit(void);
void UART_2_IntTransmit(void);
void UART_3_IntTransmit(void);

UART_DEF Uart0_Def =
{(LPC_UART_TypeDef*) LPC_UART0,	//
    {0, 0, 0, NULL, 0, 0, 0, NULL, RESET, FALSE}, //
    &UART_0_IntReceive,
    &UART_0_IntTransmit,

    0,	// rx_port;
    3,	// rx_bit;
    1,	// rx_func;

    0,	// tx_port;
    2,	// tx_bit;
    1,	// tx_func;

    STD_UART_RX_RING_BUFSIZE, //
    STD_UART_TX_RING_BUFSIZE, //

    "Uart0", //
    };

UART_DEF Uart1_Def =
{(LPC_UART_TypeDef*) LPC_UART1,
{0, 0, 0, NULL, 0, 0, 0, NULL, RESET, FALSE}, //
    &UART_1_IntReceive,
    &UART_1_IntTransmit,

    0,	// rx_port;
    16,	// rx_bit;
    1,	// rx_func;

    0,	// tx_port;
    15,	// tx_bit;
    1,	// tx_func;

    STD_UART_RX_RING_BUFSIZE,	//
    STD_UART_TX_RING_BUFSIZE,	//

    "Uart1", //
    };

UART_DEF Uart2_Def =
{(LPC_UART_TypeDef*) LPC_UART2, //
    {0, 0, 0, NULL, 0, 0, 0, NULL, RESET, FALSE}, //
    &UART_2_IntReceive,
    &UART_2_IntTransmit,

    0,	// rx_port;
    11,	// rx_bit;
    1,	// rx_func;

    0,	// tx_port;
    10,	// tx_bit;
    1,	// tx_func;

    STD_UART_RX_RING_BUFSIZE,	//
    STD_UART_TX_RING_BUFSIZE,	//

    "Uart2", //
    };

UART_DEF Uart3_Def =
{(LPC_UART_TypeDef*) LPC_UART3, //
    {0, 0, 0, NULL, 0, 0, 0, NULL, RESET, FALSE}, //
    &UART_3_IntReceive,
    &UART_3_IntTransmit,

    0,	// rx_port;
    1,	// rx_bit;
    2,	// rx_func;

    0,	// tx_port;
    0,	// tx_bit;
    2,	// tx_func;

    DEBUG_UART_RX_RING_BUFSIZE,	//
    DEBUG_UART_TX_RING_BUFSIZE,	//

    "Uart3", //
    };

UART_DEF* Uarts[UART_COUNT] =
{ &Uart0_Def, &Uart1_Def, &Uart2_Def, &Uart3_Def};

inline bool UartChValid(int ch)
{
	bool res = (ch >= 0) && (ch < UART_COUNT);

	if ( !res)
	{
		msg("ERROR: invalid UART channel <");
		p32(ch);
		msgn(">");

		msgn("HALTED!");

		while (1)
			;
	}

	return res;
}
/********************************************************************//**
 * @brief 		UART receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void STD_UART_IntReceive(int ch)
{
	if ( !UartChValid(ch))
	{
		return;
	}

	if ( !Uarts[ch]->rb.inited)
	{
		return;
	}

	u8 tmpc;
	u32 rLen;

	while (1)
	{
		// Call UART read function in UART driver
		rLen = UART_Receive(Uarts[ch]->LPC_UART, &tmpc, 1, NONE_BLOCKING);
		// If data received
		if (rLen)
		{
			/* Check if buffer is more space
			 * If no more space, remaining character will be trimmed out
			 */
			if ( !__RX_BUF_IS_FULL(Uarts[ch]->rb))
			{
				Uarts[ch]->rb.rx[Uarts[ch]->rb.rx_head] = tmpc;

				Uarts[ch]->rb.rx_head = (Uarts[ch]->rb.rx_head + 1) % Uarts[ch]->rb.rx_buf_size;
			}
		}
		// no more data
		else
		{
			break;
		}
	}
}

void UART_0_IntReceive(void)
{
	STD_UART_IntReceive(0);
}

void UART_1_IntReceive(void)
{
	STD_UART_IntReceive(1);
}

void UART_2_IntReceive(void)
{
	STD_UART_IntReceive(2);
}

void UART_3_IntReceive(void)
{
	STD_UART_IntReceive(3);
}

/********************************************************************//**
 * @brief 		UART transmit function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void STD_UART_IntTransmit(int ch)
{
	if ( !UartChValid(ch))
	{
		return;
	}

	// Disable THRE interrupt
	UART_IntConfig(Uarts[ch]->LPC_UART, UART_INTCFG_THRE, DISABLE);
	Uarts[ch]->rb.TxIntStat = RESET;

	if ( !Uarts[ch]->rb.inited)
		return;

	/* Wait for FIFO buffer empty, transfer UART_TX_FIFO_SIZE bytes
	 * of data or break whenever ring buffers are empty */
	/* Wait until THR empty */
	while (UART_CheckBusy(Uarts[ch]->LPC_UART) == SET)
		;

	if ( !__TX_BUF_IS_EMPTY(Uarts[ch]->rb))
	{
		/* Move a piece of data into the transmit FIFO */
		if (UART_Send(Uarts[ch]->LPC_UART, (u8 *) &Uarts[ch]->rb.tx[Uarts[ch]->rb.tx_tail], 1, BLOCKING))
		{
			// Update transmit ring FIFO tail pointer
			Uarts[ch]->rb.tx_tail = (Uarts[ch]->rb.tx_tail + 1) % Uarts[ch]->rb.tx_buf_size;
		}
	}

	// If there is no more data to send, disable the transmit
	// interrupt - else enable it or keep it enabled
	if (__TX_BUF_IS_EMPTY(Uarts[ch]->rb))
	{
		UART_IntConfig(Uarts[ch]->LPC_UART, UART_INTCFG_THRE, DISABLE);
		// Reset Tx Interrupt state
		Uarts[ch]->rb.TxIntStat = RESET;
	}
	else
	{
		// Set Tx Interrupt state
		Uarts[ch]->rb.TxIntStat = SET;
		UART_IntConfig(Uarts[ch]->LPC_UART, UART_INTCFG_THRE, ENABLE);
	}
}

void UART_0_IntTransmit()
{
	STD_UART_IntTransmit(0);
}

void UART_1_IntTransmit()
{
	STD_UART_IntTransmit(1);
}

void UART_2_IntTransmit()
{
	STD_UART_IntTransmit(2);
}

void UART_3_IntTransmit()
{
	STD_UART_IntTransmit(3);
}

void STD_UART_IntErr(u8 bLSErrType)
{
}

void _STD_UART_Init(int ch, i32 Baud_rate, UART_PARITY_Type parity)
{
	if ( !UartChValid(ch))
	{
		return;
	}

	PINSEL_SetPinFunc(Uarts[ch]->rx_port, Uarts[ch]->rx_bit, Uarts[ch]->rx_func);
	PINSEL_SetPinFunc(Uarts[ch]->tx_port, Uarts[ch]->tx_bit, Uarts[ch]->tx_func);

	USART_Init(Uarts[ch]->LPC_UART, Baud_rate, parity, Uarts[ch]->RxFunc, Uarts[ch]->TxFunc, STD_UART_IntErr, &Uarts[ch]->rb, Uarts[ch]->rx_buf_size, Uarts[ch]->tx_buf_size,
	           Uarts[ch]->name);
}

void std_set_baud(int ch, i32 baud_rate)
{
	if ( !UartChValid(ch))
	{
		return;
	}

	USART_SetBaud(Uarts[ch]->LPC_UART, baud_rate);
}

void STD_UART_Init(void)
{
	for (int ch = 0; ch < UART_COUNT; ch ++)
	{
		_STD_UART_Init(ch, 115200, UART_PARITY_NONE);
	}
}

u32 STD_UART_Send(int ch, const char* buf, u32 buflen)
{
	if ( !UartChValid(ch))
	{
		return 0;
	}

	return (UARTSend(Uarts[ch]->LPC_UART, (u8*) buf, buflen, &Uarts[ch]->rb, Uarts[ch]->TxFunc));
}

void std_msg_len(int ch, const char* s, u32 len)
{
	STD_UART_Send(ch, s, len);
}

void std_msg(int ch, const char* s)
{
	std_msg_len(ch, s, strlen(s));
}

// вывод в uart без вырезания и вставки \n и \r
u32 _std_sendchar(int ch, u32 c)
{
	char buf[2] = " ";
	buf[0] = c;
	STD_UART_Send(ch, buf, 1);

	return (c);
}

int std_sendchar(int ch, int c)
{
	if (c != '\r')
	{
		(void) _std_sendchar(ch, c);

		if (c == '\n')
		{
			(void) _std_sendchar(ch, '\r');
		}
	}

	return (c);
}

int std_putchar(int ch, int c)
{
	return std_sendchar(ch, c);
}

void std_get_clear(int ch)
{
	if ( !UartChValid(ch))
	{
		return;
	}

	DI();
	Uarts[ch]->rb.rx_head = Uarts[ch]->rb.rx_tail = 0;
	EI();
}

Bool std_get_available(int ch)
{
	if ( !UartChValid(ch))
	{
		return false;
	}

	return (Uarts[ch]->rb.inited && ((Uarts[ch]->rb.rx_head % Uarts[ch]->rb.rx_buf_size) != (Uarts[ch]->rb.rx_tail % Uarts[ch]->rb.rx_buf_size)));
}

u8 std_get(int ch)
{
	if ( !UartChValid(ch))
	{
		return 0;
	}

	u8 res;
	if (UARTReceive(Uarts[ch]->LPC_UART, &res, 1, &Uarts[ch]->rb) == 1)
	{
		return res;
	}
	else
	{
		return 0;
	}
}

