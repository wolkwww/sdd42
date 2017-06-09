/*
 * DS18B20.c
 *
 *  Created on: 10.09.2014
 *      Author: mrhru
 */

#include "defines.h"
#ifdef TERMO_STATION

#include <stdio.h>

#include "lpc17xx_libcfg.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_pinsel.h"

#include "types.h"
#include "common.h"
#include "HAL.h"

#include "DS18B20.h"

#define MAX_TERM_SENSORS			32
#define AVG_PERIOD					4

struct TDS18B20LowData
{
	u8 devBuf[MAX_TERM_SENSORS * 8];

	int SensCount;
	float LastValues[MAX_TERM_SENSORS][AVG_PERIOD];
	int AvgWritePos[MAX_TERM_SENSORS];
};

struct TDS18B20LowData DS18B20LowData;
bool DS18B20_Enabled = false;

float GetTemperature(int devNum);
void ReadDevTemperature(int devNum);
void SetDSValue(int devNum, float val);

void Dev_Scan(void);

void x_delay(int n)
{
	while (n --)
	{
		NOP();
	}
}

void delay_1us(void)
{
	x_delay(2);
}

void delay_us(int n)
{
	while (n --)
	{
		delay_1us();
	}
}

void delay_15us(void)
{
	delay_us(31);
}

void delay_45us(void)
{
	delay_us(98);
}

void delay_60us(void)
{
	delay_us(131);
}

void delay_480us(void)
{
	delay_us(1063);
}

// �������
#define THERM_CMD_CONVERTTEMP 		0x44
#define THERM_CMD_RSCRATCHPAD 		0xBE
#define THERM_CMD_WSCRATCHPAD 		0x4E
#define THERM_CMD_CPYSCRATCHPAD 	0x48
#define THERM_CMD_RECEEPROM 		0xB8
#define THERM_CMD_RPWRSUPPLY 		0xB4
#define THERM_CMD_SEARCHROM 		0xF0
#define THERM_CMD_READROM 			0x33
#define THERM_CMD_MATCHROM 			0x55
#define THERM_CMD_SKIPROM 			0xCC
#define THERM_CMD_ALARMSEARCH 		0xEC

#define THERM_DECIMAL_STEPS_12BIT 	625 //.0625
// ������ ������. ��-��������� 12 ���.
typedef enum
{
	THERM_MODE_9BIT = 0x1F,
	THERM_MODE_10BIT = 0x3F,
	THERM_MODE_11BIT = 0x5F,
	THERM_MODE_12BIT = 0x7F
} THERM_MODE;

/**
 * @brief ����� ������������
 * @par
 * ��������� �����������
 * @retval
 * FALSE - �������������; TRUE - �����
 */
static bool therm_reset(void)
{
	u8 i = 0xFF;

	THERM_OUTPUT_MODE();
	THERM_LOW();

	delay_480us();

	THERM_INPUT_MODE();

	delay_60us();

	i = THERM_READ();

	delay_480us();
// 0 �������� ���������� �����, 1 - ������
	return (0 == i) ? TRUE : FALSE;
}

/**
 * @brief ������ ����
 * @param[in] bBit ���
 * @retval
 * ������������ �������� �����������
 */
static void therm_write_bit(bool bBit)
{
	THERM_OUTPUT_MODE();
	THERM_LOW();

	delay_1us();

	if (bBit)
	{
		THERM_INPUT_MODE();
	}

	delay_60us();

	THERM_INPUT_MODE();
}

/**
 * @brief ������ ����
 * @par
 * ��������� �����������
 * @retval
 * �������� ����.
 */
static bool therm_read_bit(void)
{
	bool bBit = FALSE;

	THERM_OUTPUT_MODE();
	THERM_LOW();

	delay_1us();

	THERM_INPUT_MODE();

	delay_15us();

	if (THERM_READ())
	{
		bBit = TRUE;
	}

	delay_45us();

	return bBit;
}

/**
 * @brief ������ �����
 * @par
 * ��������� �����������
 * @retval
 * �������� �����.
 */
static u8 therm_read_byte(void)
{
	u8 i = 8;
	u8 n = 0;
	while (i --)
	{
		// �������� �� ���� ������� ������ � ��������� �������� ����
		n >>= 1;
		n |= (therm_read_bit() << 7);
	}
	return n;
}

/**
 * @brief ������ �����
 * @param[in] byte ����
 * @retval
 * ������������ �������� �����������
 */
static void therm_write_byte(u8 byte)
{
	u8 i = 8;

	while (i --)
	{
		// �������� ������� ��� � �������� �� 1 ������� ������
		// ��� ������� � ���������� ����
		therm_write_bit(byte & 1);
		byte >>= 1;
	}
}

/**
 * @brief ���������� ����� ������ ������������
 * @param[in] mode ����� ������
 * @retval
 * ������������ �������� �����������
 */
//static void therm_init_mode(THERM_MODE mode)
//{
//	therm_reset();
//	therm_write_byte(THERM_CMD_SKIPROM);
//	therm_write_byte(THERM_CMD_WSCRATCHPAD);
//	therm_write_byte(0);
//	therm_write_byte(0);
//	therm_write_byte(mode);
//}
/**
 * @brief ������ �����������.
 * @par
 * ��������� �����������
 * @retval
 * �����������.
 */
void ReadDevTemperature(int devNum)
{
	u8* romBuf = &DS18B20LowData.devBuf[devNum * 8];
	float res = 0;
	u8 temperature[2] =
	{0, 0};
	i16 digit;
	int iReadLimit;

// ������ ���������� �� ����� ������ �������
	DI();

// ����� � ����� ������� � �������������� �����������
	therm_reset();
	if (romBuf == NULL)
	{
		therm_write_byte(THERM_CMD_SKIPROM);
	}
	else
	{
		therm_write_byte(THERM_CMD_MATCHROM);
		for (int i = 0; i < 8; i ++)
		{
			therm_write_byte(romBuf[i]);
		}
	}
	therm_write_byte(THERM_CMD_CONVERTTEMP);

// �������� ���������� ��������������
	iReadLimit = 10;
	while ( !therm_read_bit() && ( --iReadLimit > 0))
	{
		;
	}

// ����� � ������ ���� �����������
	therm_reset();
	if (romBuf == NULL)
	{
		therm_write_byte(THERM_CMD_SKIPROM);
	}
	else
	{
		therm_write_byte(THERM_CMD_MATCHROM);
		for (int i = 0; i < 8; i ++)
		{
			therm_write_byte(romBuf[i]);
//			printf("%2X ", (int) romBuf[i]);
		}
	}
	therm_write_byte(THERM_CMD_RSCRATCHPAD);
	temperature[0] = therm_read_byte();
	temperature[1] = therm_read_byte();

// ��������� ����������
	EI();

	digit = (int) (((u16) temperature[1] << 8) + (u16) temperature[0]);

//	printf("\t<%2X-%2X>\t", (int) temperature[1], (int) temperature[0]);

//  digit  = (  temperature[0]         >> 4 ) & 0x0F;
//  digit |= ( (temperature[1] & 0x0F) << 4 ) & 0xF0;

// ������������� ����������� - ������������� � ��������� 1
//  if (temperature[1] & 0x80)
//  {
//    iResult = (~digit + 1);
//  } else
//  {
//    iResult = digit;
//  }

	res = digit / 16.0;

	SetDSValue(devNum, res);
}

// ������ �������� � ��������� �����
void SetDSValue(int devNum, float val)
{
	if (val != 85.0) // 85.0 - bad temperature value
	{
		DS18B20LowData.LastValues[devNum][DS18B20LowData.AvgWritePos[devNum]] = val;
		DS18B20LowData.AvgWritePos[devNum] = (DS18B20LowData.AvgWritePos[devNum] + 1) % AVG_PERIOD;
	}
}

float GetTemperature(int devNum)
{
	float res = 0.0;

	if (devNum <= DS18B20LowData.SensCount)
	{
//		msg("<<");
		for (int i = 0; i < AVG_PERIOD; i ++)
		{
			res += DS18B20LowData.LastValues[devNum][i];
//			printf("%.2f, ", DS18B20LowData.LastValues[devNum][i]);
		}
//		msg(">>");
	}

	return res / AVG_PERIOD;
}

//-----------------------------------------------------------------------------
// ������ ������� ������������ ������������ ���� 1-wire � ���������� ���������
//   ID ��������� � ������ buf, �� 8 ���� �� ������ ����������.
// ���������� num ������������ ���������� ��������� ���������, ����� �� �����������
// �����.
//-----------------------------------------------------------------------------
void Dev_Scan(void)
{
	int found = 0;
	u8 *lastDevice = NULL;
	u8 *curDevice = DS18B20LowData.devBuf;
	int numBit, lastCollision, currentCollision, currentSelection;

	int num = MAX_TERM_SENSORS;

	bool bit0, bit1;

	for (int i = 0; i < num * 8; i ++)
	{
		DS18B20LowData.devBuf[i] = 0;
	}

	lastCollision = 0;
	while (found < num)
	{
		numBit = 1;
		currentCollision = 0;

		// �������� ������� �� ����� ���������
		therm_reset();
		therm_write_byte(THERM_CMD_SEARCHROM);

		for (numBit = 1; numBit <= 64; numBit ++)
		{
			// ������ ��� ����. �������� � ���������������
			bit0 = therm_read_bit();
			bit1 = therm_read_bit();

			if (bit0 == TRUE)
			{
				if (bit1 == TRUE)
				{
					// ��� �������, ���-�� ���������� � ����������� �����
					DS18B20LowData.SensCount = found;
					return;
				}
				else
				{
					// 10 - �� ������ ����� ������ 1
					currentSelection = 1;
				}
			}
			else
			{
//				if (ow_buf[1] == OW_R_1)
				if (bit1 == TRUE)
				{
					// 01 - �� ������ ����� ������ 0
					currentSelection = 0;
				}
				else
				{
					// 00 - ��������
					if (numBit < lastCollision)
					{
						// ���� �� ������, �� ����� �� ��������
						if (lastDevice[(numBit - 1) >> 3] & 1 << ((numBit - 1) & 0x07))
						{
							// (numBit-1)>>3 - ����� �����
							// (numBit-1)&0x07 - ����� ���� � �����
							currentSelection = 1;

							// ���� ����� �� ������ �����, ���������� ����� ����
							if (currentCollision < numBit)
							{
								currentCollision = numBit;
							}
						}
						else
						{
							currentSelection = 0;
						}
					}
					else
					{
						if (numBit == lastCollision)
						{
							currentSelection = 0;
						}
						else
						{
							// ���� �� ������ �����
							currentSelection = 1;

							// ���� ����� �� ������ �����, ���������� ����� ����
							if (currentCollision < numBit)
							{
								currentCollision = numBit;
							}
						}
					}
				}
			}

			if (currentSelection == 1)
			{
				curDevice[(numBit - 1) >> 3] |= 1 << ((numBit - 1) & 0x07);
				therm_write_bit(TRUE);
			}
			else
			{
				curDevice[(numBit - 1) >> 3] &= ~(1 << ((numBit - 1) & 0x07));
				therm_write_bit(FALSE);
			}
		}
		found ++;
		lastDevice = curDevice;
		curDevice += 8;
		if (currentCollision == 0)
		{
			DS18B20LowData.SensCount = found;
			return;
		}

		lastCollision = currentCollision;
	}

	DS18B20LowData.SensCount = found;
}

void DS18B20_Init(void)
{
	for (int i = 0; i < MAX_TERM_SENSORS; i ++)
	{
		DS18B20LowData.SensCount = 0;
		for (int j = 0; j < AVG_PERIOD; j ++)
		{
			DS18B20LowData.LastValues[i][j] = 0;
		}
		DS18B20LowData.AvgWritePos[i] = 0;
	}

	GPIO_Init_OpenDrain(DS18B20_DATA_PORT, DS18B20_DATA_PIN, OUTPUT, TRUE);

	DS18B20_Enabled = true;
}

static u32 last_time;

void DS18B20_process(void)
{
	if (DS18B20_Enabled)
	{

		// �� ���� ���� � �������
		if (last_time == GetUptime())
		{
			return;
		}
		last_time = GetUptime();

		// ��� � 10 ������
		if ((GetUptime() % 10) == 0)
		{
			Dev_Scan();
			msg("DS18B20 [");
			p32(DS18B20LowData.SensCount);
			msg("]");
			CR();
			return;
		}

		int devNum = DS18B20LowData.SensCount;

		if (devNum > 0)
		{

//	for (int id = 0; id < devNum; id++)
//	{
//		printf("%i: ", id);
//		for (int i = 0; i < 8; i++)
//		{
//			printf("%2X ", (u32) DS18B20LowData.devBuf[id * 8 + i]);
//		}
//		float t = ReadDevTemperature(id);
//		printf(" = %.2f\t", t);
//		CR();
//	}

			msg(">>>>>>>>>>>>>>> TEMPS [");
			p32(DS18B20LowData.SensCount);
			msg("]");
			CR();

			for (int id = 0; id < devNum; id ++)
			{
				ReadDevTemperature(id);
				// test
				float t = GetTemperature(id);
				printf("%.2f\t", t);
			}
			CR();
		}
	}
}

#endif
