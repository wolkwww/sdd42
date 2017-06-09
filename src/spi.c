// spi.c

#include "LPC17xx_spi.h"
#include "spi.h"
//#include "HAL.h"

void CS_Low(void)
{
	LPC17xx_SPI_Select();
}

void CS_High(void)
{
	LPC17xx_SPI_DeSelect();
}

void InitSPI(void)
{
	LPC17xx_SPI_Init();
}

u8 SPI_ReadWriteData(u8 Data)
{
	return LPC17xx_SPI_SendRecvByte(Data);
}

void SPI_Write2Data(u8 b1, u8 b2)
{
	LPC17xx_SPI_Send2Byte(b1, b2);
}

void SPI_ReadBuffer(u8* buf, u32 len)
{
	LPC17xx_SPI_ReadBuffer(buf, len);
}

void SPI_WriteBuffer(u8* buf, u32 len)
{
	LPC17xx_SPI_WriteBuffer(buf, len);
}
