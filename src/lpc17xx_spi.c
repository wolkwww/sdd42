//LPC17xx_spi.c

#include "LPC17xx.H"                              /* LPC17xx definitions    */
#include "LPC17xx_spi.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_pinsel.h"
#include "types.h"
#include "HAL.h"
#include "common.h"

/* bit definitions for register SSPCR0. */
#define SSPCR0_DSS      0
#define SSPCR0_CPOL     6
#define SSPCR0_CPHA     7
#define SSPCR0_SCR      8
/* bit definitions for register SSPCR1. */
#define SSPCR1_SSE      1
/* bit definitions for register SSPSR. */
#define SSPSR_TFE       0
#define SSPSR_TNF       1	//used
#define SSPSR_RNE       2	//used
#define SSPSR_RFF       3
#define SSPSR_BSY       4	//used
/* Local functions */
u8 LPC17xx_SPI_SendRecvByte(u8 byte_s);

/* Initialize the SSP0, SSP0_PCLK=CCLK=72MHz */
void LPC17xx_SPI_Init(void)
{
	// Initialize and enable the SSP1 Interface module.
	LPC_SC->PCONP |= (1 << 10); /* Enable power to SSPI1 block  */

	// SCK, MISO, MOSI are SSP pins.
	GPIO_SetDir(ETHER_MOSI_Port, _BV(ETHER_MOSI_Pin), OUTPUT);
	PINSEL_SetPinFunc(ETHER_MOSI_Port, ETHER_MOSI_Pin, ETHER_MOSI_FUNC);

	PINSEL_SetPinFunc(ETHER_MISO_Port, ETHER_MISO_Pin, ETHER_MISO_FUNC);

	GPIO_SetDir(ETHER_SCK_Port, _BV(ETHER_SCK_Pin), OUTPUT);
	PINSEL_SetPinFunc(ETHER_SCK_Port, ETHER_SCK_Pin, ETHER_SCK_FUNC);

	// PCLK_SSP0=CCLK

	SSP_CFG_Type SSP_ConfigStruct;
	// initialize SSP configuration structure to default
	SSP_ConfigStructInit(&SSP_ConfigStruct);

	SSP_ConfigStruct.CPHA = SSP_CPHA_FIRST;
	SSP_ConfigStruct.CPOL = SSP_CPOL_HI;
	SSP_ConfigStruct.ClockRate = 16000000UL;
	SSP_ConfigStruct.Databit = SSP_DATABIT_8;
	SSP_ConfigStruct.Mode = SSP_MASTER_MODE;
	SSP_ConfigStruct.FrameFormat = SSP_FRAME_SPI;

	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);

	// SSEL is GPIO, output set to high.
	GPIO_SetDir(ETHER_CE_Port, _BV(ETHER_CE_Pin), OUTPUT);
	PINSEL_SetPinFunc(ETHER_CE_Port, ETHER_CE_Pin, ETHER_CE_FUNC);
	LPC17xx_SPI_DeSelect(); // set P0.6 high (SSEL inactiv)

	// wait for busy gone
	while (LPC_SSP1->SR & (1 << SSPSR_BSY))
		;

	// drain SPI RX FIFO
	u32 dummy;
	while (LPC_SSP1->SR & (1 << SSPSR_RNE))
	{
		dummy = LPC_SSP1->DR;
	}
	(void) dummy;

	NVIC_SetPriorityGrouping(0x05);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(SSP1_IRQn, ((0x01<<3)|0x01));
    /* Enable SSP0 interrupt */
    NVIC_EnableIRQ(SSP1_IRQn);

}

/* Close SSP1 */
void LPC17xx_SPI_DeInit(void)
{
	SSP_DeInit(LPC_SSP1);
}

// SSEL: low
void LPC17xx_SPI_Select(void)
{
	GPIO_ClearValue(ETHER_CE_Port, _BV(ETHER_CE_Pin));
}
// SSEL: high
void LPC17xx_SPI_DeSelect(void)
{
	GPIO_SetValue(ETHER_CE_Port, _BV(ETHER_CE_Pin));
//	delay(1);
}

// Release SSP0
void LPC17xx_SPI_Release(void)
{
	LPC17xx_SPI_DeSelect();
	LPC17xx_SPI_RecvByte();
}

////////////////////////////////////////////////////////////////
// Send one byte
void LPC17xx_SPI_SendByte(u8 data)
{
	LPC17xx_SPI_SendRecvByte(data);
}

// Recv one byte
u8 LPC17xx_SPI_RecvByte()
{
	return (LPC17xx_SPI_SendRecvByte(0xFF));
}

////////////////////////////////////////////////////////////////
void SSP1_IRQHandler(void)
{
	SSP1_StdIntHandler();
}

__IO FlagStatus complete;

// User SPI callback function
void SSPCallBack(void)
{
	// Set Complete Flag
	complete = SET;
}

//u8 LPC17xx_SPI_SendRecvByte(u8 byte_s)
//{
//	u8 byte_r = 0;
//
//	LPC_SSP1->DR = byte_s;
//	while (LPC_SSP1->SR & (1 << SSPSR_BSY) /*BSY*/)
//	{
//		byte_r = LPC_SSP1->DR;
//	}
//
//	return (byte_r);
//}

SSP_DATA_SETUP_Type xferConfig;
u8 LPC17xx_SPI_SendRecvByte(u8 tx_b)
{
	u8 rx_b;

	xferConfig.tx_data = &tx_b;
	xferConfig.rx_data = &rx_b;
	xferConfig.length = sizeof (u8);
	xferConfig.callback = SSPCallBack;
	SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_INTERRUPT);
	while (complete == RESET);

	return (rx_b);
}

void LPC17xx_SPI_Send2Byte(u8 b1, u8 b2)
{
	u8 tx[2]; tx[0] = b1; tx[1] = b2;
	u8 rx[2];

	xferConfig.tx_data = tx;
	xferConfig.rx_data = rx;
	xferConfig.length = sizeof (tx);
	xferConfig.callback = SSPCallBack;
	SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_INTERRUPT);
	while (complete == RESET);
}

void LPC17xx_SPI_ReadBuffer(u8* buf, u32 len)
{
	xferConfig.tx_data = NULL;
	xferConfig.rx_data = buf;
	xferConfig.length = len;
	xferConfig.callback = SSPCallBack;
	SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_INTERRUPT);
	while (complete == RESET);
}

void LPC17xx_SPI_WriteBuffer(u8* buf, u32 len)
{
	xferConfig.tx_data = buf;
	xferConfig.rx_data = NULL;
	xferConfig.length = len;
	xferConfig.callback = SSPCallBack;
	SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_INTERRUPT);
	while (complete == RESET);
}

