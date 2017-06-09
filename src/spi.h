// spi.h
 
#ifndef _SPI_H
#define _SPI_H

#include "types.h"

#define SPI_NOT_INIT	0x00
#define SPI_HALF_SPEED	0x01
#define SPI_FULL_SPEED	0x02

void InitSPI(void);
u8 SPI_ReadWriteData(u8 Data);
void SPI_Write2Data(u8 b1, u8 b2);
void SPI_ReadBuffer(u8* buf, u32 len);
void SPI_WriteBuffer(u8* buf, u32 len);


void CS_Low(void);
void CS_High(void);
	

#endif


