/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */

#include <stdio.h>

#include "HAL.h"
//#include "spi.h"
//#include "stm32f3xx_hal_spi.h"
#include "tm_stm32f3_nrf24l01.h"
#include "common.h"

//#define SPI_TIMEOUT			100
//#define NRF24L01_SPI		(&hspi1)

void x(int n)
{
	printf("--<%d>--\n\r", n);
}

bool TestNRFRegs(void)
{
	for (u8 reg = 0; reg < 32; reg ++)
	{
		u8 val = TM_NRF24L01_ReadRegister(reg);
		if (val == 0xFF)
		{
			return false;
		}
	}
	return true;
}

void ShowRegs(void)
{
	msgn("------nRF Regs-----");
	for (u8 reg = 0; reg < 32; reg ++)
	{
		u8 val = TM_NRF24L01_ReadRegister(reg);
		printf("%.2X:\t%.2X\n\r", (int) reg, (int) val);
	}
	CR();
	msgn("-------------------");
}

void TM_SPI_Delay(void)
{
//	for(int i =0; i < 100; i++)
//	{
//		;
//	}
}

u8 TM_SPI_Send(u8 ch)
{
	u8 mask, res = 0;
	for (mask = 0x80; mask != 0; mask >>= 1)
	{
		// output
		if ((ch & mask) != 0)
		{
			NRF24L01_MOSI_HIGH();
		}
		else
		{
			NRF24L01_MOSI_LOW();
		}
		// input
		if (NRF24L01_MISO_READ() != 0)
		{
			res |= mask;
		}

//		TM_SPI_Delay();
		NRF24L01_SCK_HIGH();
//		TM_SPI_Delay();
		NRF24L01_SCK_LOW();
	}

	return res;
}

void TM_SPI_WriteMulti(u8* dataOut, u16 count)
{
	int i;
	for (i = 0; i < count; i ++)
	{
		TM_SPI_Send( *dataOut ++);
	}
}

void TM_SPI_ReadMulti(u8* dataIn, u8 dummy, u16 count)
{
	int i;
	for (i = 0; i < count; i ++)
	{
		*dataIn ++ = TM_SPI_Send(dummy);
	}
}

void TM_SPI_SendMulti(u8 *dataOut, u8 *dataIn, u16 count)
{
	u16 i;
	for (i = 0; i < count; i ++)
	{
		*dataIn ++ = TM_SPI_Send( *dataOut ++);
	}
}

TM_NRF24L01_t TM_NRF24L01_Struct;

void TM_NRF24L01_InitPins(void)
{
	GPIO_Init(NRF24L01_CE_PORT, NRF24L01_CE_PIN, OUTPUT, true);
	GPIO_Init(NRF24L01_CSN_PORT, NRF24L01_CSN_PIN, OUTPUT, true);
	GPIO_Init(NRF24L01_SCK_PORT, NRF24L01_SCK_PIN, OUTPUT, true);
	GPIO_Init(NRF24L01_MOSI_PORT, NRF24L01_MOSI_PIN, OUTPUT, true);

	GPIO_Init_Input_PullUp(NRF24L01_MISO_PORT, NRF24L01_MISO_PIN);
	GPIO_Init_Input_PullUp(NRF24L01_IRQ_PORT, NRF24L01_IRQ_PIN);

	NRF24L01_CE_LOW();
	NRF24L01_CSN_HIGH();
}

u8 TM_NRF24L01_Init(u8 channel, u8 payload_size, u8 autoRetransmitCount)
{
	//Initialize CE and CSN pins
	TM_NRF24L01_InitPins();

	//Max payload is 32bytes
	if (payload_size > 32)
	{
		payload_size = 32;
	}

	TM_NRF24L01_Struct.Channel = channel;
	TM_NRF24L01_Struct.PayloadSize = payload_size;
	TM_NRF24L01_Struct.OutPwr = TM_NRF24L01_OutputPower_0dBm;
	TM_NRF24L01_Struct.DataRate = TM_NRF24L01_DataRate_2M;

	//Reset nRF24L01+ to power on registers values
	TM_NRF24L01_SoftwareReset();

	//Channel select
	TM_NRF24L01_SetChannel(TM_NRF24L01_Struct.Channel);

	//Set pipeline to max possible 32 bytes
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, TM_NRF24L01_Struct.PayloadSize); // Auto-ACK pipe
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, TM_NRF24L01_Struct.PayloadSize); // Data payload pipe
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, TM_NRF24L01_Struct.PayloadSize);

	//Set RF settings (2mbps, output power)
	TM_NRF24L01_SetRF(TM_NRF24L01_Struct.DataRate, TM_NRF24L01_Struct.OutPwr);

	//Config register
//	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG_NO_CRC);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG);

	//Enable auto-acknowledgment for all pipes
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_AA, 0x3F);

	//Enable RX addresses
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, 0x3F);

	//Auto retransmit delay: 1000 (4*250 + 250) us and Up to 15 retransmit trials
	//TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 0x40 | autoRetransmitCount); // 0x4F);

	//Auto retransmit delay: 0000 (0*250 + 250) us and disable retransmit trials
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 0x40 | autoRetransmitCount);

	//Dynamic length configurations: No dynamic length
	TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD,
	                          (0 << NRF24L01_DPL_P0) |
	                          (0 << NRF24L01_DPL_P1) |
	                          (0 << NRF24L01_DPL_P2) |
	                          (0 << NRF24L01_DPL_P3) |
	                          (0 << NRF24L01_DPL_P4) |
	                          (0 << NRF24L01_DPL_P5));

	//Clear FIFOs
	NRF24L01_FLUSH_TX()
	;
	NRF24L01_FLUSH_RX()
	;

	//Go to RX mode
	TM_NRF24L01_PowerUpRx();

	return 1;
}

void TM_NRF24L01_SetMyAddress(u8 *adr)
{
	NRF24L01_CE_LOW();
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, adr, 5);
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, adr, 5);
	NRF24L01_CE_HIGH();
}

void TM_NRF24L01_SetTxAddress(u8 *adr)
{
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, adr, 5);
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, adr, 5);
}

void TM_NRF24L01_SetPipe2Address(u8 adr)
{
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P2, adr);
}

void TM_NRF24L01_SetPipe3Address(u8 adr)
{
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P3, adr);
}

void TM_NRF24L01_SetPipe4Address(u8 adr)
{
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P4, adr);
}

void TM_NRF24L01_SetPipe5Address(u8 adr)
{
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P5, adr);
}

void TM_NRF24L01_WriteBit(u8 reg, u8 bit, BitAction value)
{
	u8 tmp;
	tmp = TM_NRF24L01_ReadRegister(reg);
	if (value != Bit_RESET)
	{
		tmp |= 1 << bit;
	}
	else
	{
		tmp &= ~(1 << bit);
	}
	TM_NRF24L01_WriteRegister(reg, tmp);
}

u8 TM_NRF24L01_ReadBit(u8 reg, u8 bit)
{
	u8 tmp;
	tmp = TM_NRF24L01_ReadRegister(reg);
	if ( !NRF24L01_CHECK_BIT(tmp, bit))
	{
		return 0;
	}
	return 1;
}

u8 TM_NRF24L01_ReadRegister(u8 reg)
{
	u8 value;
	NRF24L01_CSN_LOW();
	TM_SPI_Send(NRF24L01_READ_REGISTER_MASK(reg));
	value = TM_SPI_Send(NRF24L01_NOP_MASK);
	NRF24L01_CSN_HIGH();

	return value;
}

void TM_NRF24L01_ReadRegisterMulti(u8 reg, u8* data, u8 count)
{
	NRF24L01_CSN_LOW();
	TM_SPI_Send(NRF24L01_READ_REGISTER_MASK(reg));
	TM_SPI_ReadMulti(data, NRF24L01_NOP_MASK, count);
	NRF24L01_CSN_HIGH();
}

void TM_NRF24L01_WriteRegister(u8 reg, u8 value)
{
	NRF24L01_CSN_LOW();
	TM_SPI_Send(NRF24L01_WRITE_REGISTER_MASK(reg));
	TM_SPI_Send(value);
	NRF24L01_CSN_HIGH();
}

void TM_NRF24L01_WriteRegisterMulti(u8 reg, u8 *data, u8 count)
{
	NRF24L01_CSN_LOW();
	TM_SPI_Send(NRF24L01_WRITE_REGISTER_MASK(reg));
	TM_SPI_WriteMulti(data, count);
	NRF24L01_CSN_HIGH();
}

void TM_NRF24L01_PowerUpTx(void)
{
	NRF24L01_Clear_Interrupts();

	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG,
	                          NRF24L01_CONFIG_NO_CRC |
	                          (0 << NRF24L01_PRIM_RX) | (1 << NRF24L01_PWR_UP));
}

void TM_NRF24L01_PowerUpRx(void)
{
	NRF24L01_FLUSH_RX()
	;

	NRF24L01_Clear_Interrupts();

	NRF24L01_CE_LOW();

	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG,
	                          NRF24L01_CONFIG_NO_CRC |
	                          (1 << NRF24L01_PRIM_RX) | (1 << NRF24L01_PWR_UP));

	NRF24L01_CE_HIGH();
}

void TM_NRF24L01_SetBits(u8 reg, u8 bits)
{
	TM_NRF24L01_WriteRegister(reg, 0x2E | bits);
}

//Clear interrupt flags
void NRF24L01_Clear_Interrupts(void)
{
//	TM_NRF24L01_WriteBit(7, 4, SET);
//	TM_NRF24L01_WriteBit(7, 5, SET);
//	TM_NRF24L01_WriteBit(7, 6, SET);

	TM_NRF24L01_SetBits(7, (1 << 4) | (1 << 5) | (1 << 6));
}

void TM_NRF24L01_PowerDown(void)
{
	NRF24L01_CE_LOW();
	TM_NRF24L01_WriteBit(NRF24L01_REG_CONFIG, NRF24L01_PWR_UP, Bit_RESET);
	NRF24L01_CE_HIGH();

	// very impotent!!!
	NRF24L01_Clear_Interrupts();

	NRF24L01_CSN_LOW();
}

void TM_NRF24L01_Transmit(u8 *data)
{
	u8 count = TM_NRF24L01_Struct.PayloadSize;

	//Chip enable put to low, disable it
	NRF24L01_CE_LOW();

	//Go to power up tx mode
	TM_NRF24L01_PowerUpTx();

	//Clear TX FIFO from NRF24L01+
	NRF24L01_FLUSH_TX()
	;

	//Send payload to nRF24L01+
	NRF24L01_CSN_LOW();
	//Send write payload command
	TM_SPI_Send(NRF24L01_W_TX_PAYLOAD_MASK);
	//Fill payload with data
	TM_SPI_WriteMulti(data, count);
	NRF24L01_CSN_HIGH();

	//Delay(1000);
	//Send data!
	NRF24L01_CE_HIGH();
}

void TM_NRF24L01_GetData(u8* data)
{
	//Pull down chip select
	NRF24L01_CSN_LOW();
	//Send read payload command
	TM_SPI_Send(NRF24L01_R_RX_PAYLOAD_MASK);
	//Read payload
	TM_SPI_SendMulti(data, data, TM_NRF24L01_Struct.PayloadSize);
	//Pull up chip select
	NRF24L01_CSN_HIGH();

	//Reset status register, clear RX_DR interrupt flag
	TM_NRF24L01_WriteRegister(NRF24L01_REG_STATUS, (1 << NRF24L01_RX_DR));
}

bool TM_NRF24L01_DataReady(void)
{
	u8 status = TM_NRF24L01_GetStatus();

	if (NRF24L01_CHECK_BIT(status, NRF24L01_RX_DR))
	{
		return true;
	}
	return !TM_NRF24L01_RxFifoEmpty();
}

u8 TM_NRF24L01_RxFifoEmpty(void)
{
	u8 reg = TM_NRF24L01_ReadRegister(NRF24L01_REG_FIFO_STATUS);
	return NRF24L01_CHECK_BIT(reg, NRF24L01_RX_EMPTY);
}

u8 TM_NRF24L01_GetStatus(void)
{
	u8 status;

	NRF24L01_CSN_LOW();
	//First received byte is always status register
	status = TM_SPI_Send(NRF24L01_NOP_MASK);
	//Pull up chip select
	NRF24L01_CSN_HIGH();

	return status;
}

TM_NRF24L01_Transmit_Status_t TM_NRF24L01_GetTransmissionStatus(void)
{
	u8 status = TM_NRF24L01_GetStatus();

	if (NRF24L01_CHECK_BIT(status, NRF24L01_TX_DS))
	{
		//Successfully sent
		return TM_NRF24L01_Transmit_Status_Ok;
	}
	else if (NRF24L01_CHECK_BIT(status, NRF24L01_MAX_RT))
	{
		//Message lost
		return TM_NRF24L01_Transmit_Status_Lost;
	}

	//Still sending
	return TM_NRF24L01_Transmit_Status_Sending;
}

void TM_NRF24L01_SoftwareReset(void)
{
	u8 data[5];

	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_REG_DEFAULT_VAL_CONFIG);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_AA, NRF24L01_REG_DEFAULT_VAL_EN_AA);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, NRF24L01_REG_DEFAULT_VAL_EN_RXADDR);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_AW, NRF24L01_REG_DEFAULT_VAL_SETUP_AW);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, NRF24L01_REG_DEFAULT_VAL_SETUP_RETR);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, NRF24L01_REG_DEFAULT_VAL_RF_CH);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_SETUP, NRF24L01_REG_DEFAULT_VAL_RF_SETUP);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_STATUS, NRF24L01_REG_DEFAULT_VAL_STATUS);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_OBSERVE_TX, NRF24L01_REG_DEFAULT_VAL_OBSERVE_TX);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RPD, NRF24L01_REG_DEFAULT_VAL_RPD);

	//P0
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, data, 5);

	//P1
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, data, 5);

	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P2, NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P2);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P3, NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P3);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P4, NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P4);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P5, NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P5);

	//TX
	data[0] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, data, 5);

	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, NRF24L01_REG_DEFAULT_VAL_RX_PW_P0);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, NRF24L01_REG_DEFAULT_VAL_RX_PW_P1);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, NRF24L01_REG_DEFAULT_VAL_RX_PW_P2);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, NRF24L01_REG_DEFAULT_VAL_RX_PW_P3);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, NRF24L01_REG_DEFAULT_VAL_RX_PW_P4);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, NRF24L01_REG_DEFAULT_VAL_RX_PW_P5);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FIFO_STATUS, NRF24L01_REG_DEFAULT_VAL_FIFO_STATUS);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, NRF24L01_REG_DEFAULT_VAL_DYNPD);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FEATURE, NRF24L01_REG_DEFAULT_VAL_FEATURE);
}

u8 TM_NRF24L01_GetRetransmissionsCount(void)
{
	//Low 4 bits
	return TM_NRF24L01_ReadRegister(NRF24L01_REG_OBSERVE_TX) & 0x0F;
}

void TM_NRF24L01_SetChannel(u8 channel)
{
	if (channel <= 125)	// && channel != TM_NRF24L01_Struct.Channel)
	{
		TM_NRF24L01_Struct.Channel = channel;
		TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, channel);
	}
}

void TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_t DataRate, TM_NRF24L01_OutputPower_t OutPwr)
{
	u8 tmp = 0;
	TM_NRF24L01_Struct.DataRate = DataRate;
	TM_NRF24L01_Struct.OutPwr = OutPwr;

	if (DataRate == TM_NRF24L01_DataRate_2M)
	{
		tmp |= 1 << NRF24L01_RF_DR_HIGH;
	}
	else if (DataRate == TM_NRF24L01_DataRate_250k)
	{
		tmp |= 1 << NRF24L01_RF_DR_LOW;
	}
	//If 1Mbps, all bits set to 0

	if (OutPwr == TM_NRF24L01_OutputPower_0dBm)
	{
		tmp |= 3 << NRF24L01_RF_PWR;
	}
	else if (OutPwr == TM_NRF24L01_OutputPower_M6dBm)
	{
		tmp |= 2 << NRF24L01_RF_PWR;
	}
	else if (OutPwr == TM_NRF24L01_OutputPower_M12dBm)
	{
		tmp |= 1 << NRF24L01_RF_PWR;
	}

	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_SETUP, tmp);
}

