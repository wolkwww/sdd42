#ifndef __LPC17XX_SPI_H__
#define __LPC17XX_SPI_H__

#include <stdint.h>
#include <stdbool.h>

#include "types.h"

// if not use FIFO, 	R: 600kB/s, W: 500kB/s
// if     use FIFO,   	R: 1.2MB/s, W: 800kB/s 
#define USE_FIFO        1

/* bit-frequency = PCLK / (CPSDVSR * [SCR+1]), here SCR=0, PCLK=72MHz, must be even	*/
#define SPI_SPEED_20MHz		4	/* => 18MHz */
#define SPI_SPEED_25MHz		4	/* => 18MHz */
#define SPI_SPEED_400kHz  180	/* => 400kHz */

/* external functions */
void LPC17xx_SPI_Init(void);
void LPC17xx_SPI_DeInit(void);
void LPC17xx_SPI_Release(void);
void LPC17xx_SPI_SetSpeed(u8 speed);
void LPC17xx_SPI_Select(void);
void LPC17xx_SPI_DeSelect(void);

void LPC17xx_SPI_SendByte(u8 data);
u8 LPC17xx_SPI_SendRecvByte(u8 byte_s);
u8 LPC17xx_SPI_RecvByte(void);
void LPC17xx_SPI_Send2Byte(u8 b1, u8 b2);
void LPC17xx_SPI_ReadBuffer(u8* buf, u32 len);
void LPC17xx_SPI_WriteBuffer(u8* buf, u32 len);

#if USE_FIFO
void LPC17xx_SPI_RecvBlock_FIFO(u8 *buff, u32 btr);
void LPC17xx_SPI_SendBlock_FIFO(const u8 *buff);
#endif

#endif	// __LPC17XX_SPI_H__
