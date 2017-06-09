// HP03M.c

#include <stdio.h>

#include "lpc17xx_libcfg.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_pinsel.h"

#include "types.h"
#include "HP03M.h"
#include "common.h"
#include "HAL.h"

#include "udp.h"
#include "test_udp.h"
#include "transport.h"
#include "transport_low.h"
#include "GasMeters.h"

///////////////////////////////////////////////////////////////////////////
// 1
#define XCLR1_PORT           	(1)
#define XCLR1_PIN				(0)

#define SDA1_PORT           	(2)
#define SDA1_PIN				(6)

#define SCK1_PORT           	(1)
#define SCK1_PIN				(1)

// 2
#define XCLR2_PORT           	(1)
#define XCLR2_PIN				(4)

#define SDA2_PORT           	(2)
#define SDA2_PIN				(7)

#define SCK2_PORT           	(1)
#define SCK2_PIN				(8)

// 3
#define XCLR3_PORT           	(1)
#define XCLR3_PIN				(9)

#define SDA3_PORT           	(2)
#define SDA3_PIN				(8)

#define SCK3_PORT           	(1)
#define SCK3_PIN				(10)

// 4
#define XCLR4_PORT           	(1)
#define XCLR4_PIN				(14)

#define SDA4_PORT           	(2)
#define SDA4_PIN				(9)

#define SCK4_PORT           	(1)
#define SCK4_PIN				(15)

u32 SensCount = 4;

u32 php = 0; // ��������� �� ������� ������

u32 XCLR_PORT[MAX_SENS_COUNT] =
{ XCLR1_PORT, XCLR2_PORT, XCLR3_PORT, XCLR4_PORT };
u32 XCLR_PIN[MAX_SENS_COUNT] =
{ XCLR1_PIN, XCLR2_PIN, XCLR3_PIN, XCLR4_PIN };

u32 SDA_PORT[MAX_SENS_COUNT] =
{ SDA1_PORT, SDA2_PORT, SDA3_PORT, SDA4_PORT };
u32 SDA_PIN[MAX_SENS_COUNT] =
{ SDA1_PIN, SDA2_PIN, SDA3_PIN, SDA4_PIN };

u32 SCK_PORT[MAX_SENS_COUNT] =
{ SCK1_PORT, SCK2_PORT, SCK3_PORT, SCK4_PORT };
u32 SCK_PIN[MAX_SENS_COUNT] =
{ SCK1_PIN, SCK2_PIN, SCK3_PIN, SCK4_PIN };

#define XCLR_H()				GPIO_SetValue(XCLR_PORT[php], _BV(XCLR_PIN[php]))
#define XCLR_L()				GPIO_ClearValue(XCLR_PORT[php], _BV(XCLR_PIN[php]))

#define SCK_H()					GPIO_SetValue(SCK_PORT[php], _BV(SCK_PIN[php]))
#define SCK_L()					GPIO_ClearValue(SCK_PORT[php], _BV(SCK_PIN[php]))

#define SDA_H()					GPIO_SetValue(SDA_PORT[php], _BV(SDA_PIN[php]))
#define SDA_L()					GPIO_ClearValue(SDA_PORT[php], _BV(SDA_PIN[php]))
#define SDA_IN()				GPIO_Read	(SDA_PORT[php], _BV(SDA_PIN[php]))
#define SDA_SET_INPUT()			GPIO_SetDir	(SDA_PORT[php], _BV(SDA_PIN[php]), INPUT)

void SDA_SET_OUTPUT(bool output_state)
{
	if (output_state)
	{
		SDA_H();
	}
	else
	{
		SDA_L();
	}
	GPIO_SetDir(SDA_PORT[php], _BV(SDA_PIN[php]), OUTPUT);
}

void InitPWM(void);

void HP03MLowInit(void)
{
	InitPWM();

	msgn("Init HP03M");

	SensCount = 4;

	for (int p = 0; p <= SensCount - 1; p++)
	{
		php = p;

		printf("%u:\t{%u, %u}\t{%u, %u}\t{%u, %u}\n", p, XCLR_PORT[p], XCLR_PIN[p], SDA_PORT[p], SDA_PIN[p],
		SCK_PORT[p], SCK_PIN[p]);

		GPIO_Init(XCLR_PORT[p], XCLR_PIN[p], OUTPUT, false);
		GPIO_Init_OpenDrain(SDA_PORT[p], SDA_PIN[p], OUTPUT, false);
		GPIO_Init(SCK_PORT[p], SCK_PIN[p], OUTPUT, false);

		SDA_SET_OUTPUT(SET);

		Sens[p].H_Press_Offset = Sens[p].H_Temp_Offset = 0;
	}

	php = 2;
}

void InitPWM(void)
{
	const u32 F = 32768UL; // 32 kHz
	const u32 Nper = 100000000UL / 4UL / F; // 500
	const u32 Nduty = Nper / 2; // 250
	const u32 PWMChannel = 2; // ����� ������ PWM

	// PWM block section --------------------------------------------
	// Initialize PWM peripheral, timer mode
	// PWM prescale value = 1 (absolute value - tick value)
	PWM_TIMERCFG_Type PWMCfgDat;
	PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
	PWMCfgDat.PrescaleValue = 1;
	PWM_Init(LPC_PWM1, PWM_MODE_TIMER, (void *) &PWMCfgDat);

	// Initialize PWM pin connect
	PINSEL_CFG_Type PinCfg;
	PinCfg.Portnum = 3;
	PinCfg.Pinnum = 25;
	PinCfg.Funcnum = 3;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&PinCfg);

	//* Set match value for PWM match channel 0 = 256, update immediately
	PWM_MatchUpdate(LPC_PWM1, 0, Nper, PWM_MATCH_UPDATE_NOW);
	// PWM Timer/Counter will be reset when channel 0 matching
	// no interrupt when match
	// no stop when match
	PWM_MATCHCFG_Type PWMMatchCfgDat;
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = 2;
	PWMMatchCfgDat.ResetOnMatch = ENABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);

	// Configure each PWM channel: ---------------------------------------------
	// - Single edge
	// - PWM Duty on each PWM channel determined by
	// the match on channel 0 to the match of that match channel.
	// Example: PWM Duty on PWM channel 1 determined by
	// the match on channel 0 to the match of match channel 1.

	// Configure PWM channel edge option
	// Note: PWM Channel 1 is in single mode as default state and
	// can not be changed to double edge mode
	for (int ch = 2; ch < 7; ch++)
	{
		PWM_ChannelConfig(LPC_PWM1, ch, PWM_CHANNEL_SINGLE_EDGE);
	}

	// Configure match value for each match channel
	// Set up match value
	PWM_MatchUpdate(LPC_PWM1, PWMChannel, Nduty, PWM_MATCH_UPDATE_NOW);
	// Configure match option
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = PWMChannel;
	PWMMatchCfgDat.ResetOnMatch = DISABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
	// Enable PWM Channel Output
	PWM_ChannelCmd(LPC_PWM1, PWMChannel, ENABLE);

	// Reset and Start counter
	PWM_ResetCounter(LPC_PWM1);
	PWM_CounterCmd(LPC_PWM1, ENABLE);

	// Start PWM now
	PWM_Cmd(LPC_PWM1, ENABLE);
}

void I2CSoftInit(void)
{
}

u32 Dummy(u32 x)
{
	return (x * 2);
}

static volatile u32 dummy = 0;
u32 i2cdelay(void)
{
//	i = 256; // ��� i = 8 - �� �������� �� STM8S, �� ��������� ADC (������ XCLR �� ��������)
	//i = 8; // 32 ���� ������� ������ - ����������� �������� �� ������� ������

//	for (int i = 0; i < 2; i++)
//	{
//		dummy += Dummy(i) + Dummy(2 * i) + Dummy(3 * i) + Dummy(4 * i);
//	}

//	for (int i = 0; i < 2; i++)
//	{
//		dummy += Dummy(i);// + Dummy(2 * i);// + Dummy(3 * i) + Dummy(4 * i);
//	}

	u32 dummy = 0;
	for (int i = 0; i < 20; i++)
	{
		dummy += Dummy(i) + Dummy(2 * i) + Dummy(3 * i) + Dummy(4 * i);
	}

	return dummy;
}

void I2CS_Start(void)
{
	SDA_H();
	SCK_H();
	i2cdelay();
	SDA_L();
	i2cdelay();
	SCK_L();
	i2cdelay();
}

void I2CS_Stop(void)
{
	SDA_L();
	i2cdelay();
	SCK_H();
	i2cdelay();
	SDA_H();
	i2cdelay();
}

u8 I2CS_SendByte(u8 b)
{
	u8 i;
	u8 res;

	SCK_L();
	i2cdelay();	//??

	for (i = 0; i < 8; i++)
	{
		if ((b & 0x80) != 0)
		{
			SDA_H();
		}
		else
		{
			SDA_L();
		}
		b <<= 1;

		i2cdelay();
		SCK_H();
		i2cdelay();
		SCK_L();
		//i2cdelay();
	}

	SDA_SET_INPUT();
	i2cdelay();
	SCK_H();
	i2cdelay();
	res = SDA_IN();
	i2cdelay();	//??
	SDA_SET_OUTPUT(RESET);
	SCK_L();
	i2cdelay();

	return (res);
}

u8 I2CS_ReceiveByte(FlagStatus MasterAcknowledge)
{
	u8 i;
	u8 res = 0;

	SCK_L();
	SDA_SET_INPUT();

	for (i = 0; i < 8; i++)
	{
		res <<= 1;

		i2cdelay();
		SCK_H();
		i2cdelay();
		if (SDA_IN() != RESET)
		{
			//printf("ONE ");
			res |= 1;
		}

		SCK_L();
	}
	//printf("\n");

	i2cdelay();
	if (MasterAcknowledge != RESET)
	{
		SDA_SET_OUTPUT(RESET);
	}
	else
	{
		SDA_SET_OUTPUT(SET);
	}

	i2cdelay();	//??
	SCK_H();
	i2cdelay();
	SCK_L();
	i2cdelay();

	SDA_SET_OUTPUT(RESET);
	i2cdelay();

	return (res);
}

#define PressureMeasure		0xF0
#define TemperatureMeasure	0xE8

u16 HP03M_ReadADC(u8 adc_type);

//u16 HP03M_ReadADC(u8 adc_type)
//{
//	u16 msb, lsb;
//
//	XCLR_H();
//	delay(10);
//	I2CS_Start();
//	I2CS_SendByte(0xEE);
//	I2CS_SendByte(0xFF);
//	I2CS_SendByte(adc_type);
//	I2CS_Stop();
//
//	delay(50);
//
//	I2CS_Start();
//	I2CS_SendByte(0xEE);
//	I2CS_SendByte(0xFD);
//
//	I2CS_Start();
//	I2CS_SendByte(0xEF);
//
//	msb = I2CS_ReceiveByte(SET);
//	//I2CS_MasterAcknowledge();
//	lsb = I2CS_ReceiveByte(RESET);
//	//I2CS_NoMasterAcknowledge();
//	I2CS_Stop();
//
//	XCLR_L();
//	delay(10);
//
//	return ((msb * 256) + lsb);
//}
//

static u64 LDelayCounter;

void LDelay(u32 ms)
{
	LDelayCounter = GetTicks() / 10 + ms;
}

bool IfInDelay(void)
{
	if (LDelayCounter != 0)
	{
		if ((GetTicks() / 10) > LDelayCounter)
		{
			LDelayCounter = 0;
			return false;
		}
		return true;
	}
	return false;
}

#define Next()				stage++; break
#define Goto(new_stage)		stage = new_stage

u16 HP03M_ReadADC(u8 adc_type)
{
	typedef enum T_Stage
	{
		A0 = 0,
		A1,
		A2,
		A3,
//		A4,
//		A5,
//		A6,
		A7,
		A8
	} TStage;

	static TStage stage = A0;

	static u16 msb, lsb;

	u16 res = (u16) -2; // -2 ������� ��������������� ��������, -1 - ���. ������
	                    // ��� ��� ��� ���������� ���������� ������� �����������
	                    // ��� ��� -1

	switch (stage)
	{
		case A0:
			XCLR_H();
			LDelay(1);	                    //(10);

			Next()
;			case A1:
			I2CS_Start();
			I2CS_SendByte(0xEE);
			I2CS_SendByte(0xFF);
			I2CS_SendByte(adc_type);
			I2CS_Stop();

			Next();

			case A2:
			LDelay(40);	                    //(80);
			Next();

			case A3:
			I2CS_Start();
			I2CS_SendByte(0xEE);
			I2CS_SendByte(0xFD);

			I2CS_Start();
			I2CS_SendByte(0xEF);
			msb = I2CS_ReceiveByte(SET);
			lsb = I2CS_ReceiveByte(RESET);
			I2CS_Stop();
			XCLR_L();
			Next();

			case A7:
			LDelay(1);//(10);
			Next();

			case A8:
			res = ((msb * 256) + lsb);
			stage = A0;
			break;
		}

	return res;
}

u16 HP03M_PressureADC(void)
{
	return HP03M_ReadADC(PressureMeasure);
}

u16 HP03M_TemperatureADC(void)
{
	return HP03M_ReadADC(TemperatureMeasure);
}

float fabs(float a)
{
	return (a < 0.0) ? -a : a;
}

void AvgAdd(AvgBufDef* pbuf, float d)
{
	if (fabs(d - pbuf->Avg) / pbuf->Avg > 0.02)
	{
		pbuf->Avg = d;
		pbuf->AvgSum = d * AVG_BUF_SIZE;
		for (int i = 0; i < AVG_BUF_SIZE; i++)
		{
			pbuf->AvgBuf[i] = d;
		}
	}
	else
	{
		pbuf->AvgSum -= pbuf->AvgBuf[pbuf->AvgPos];
		pbuf->AvgBuf[pbuf->AvgPos] = d;
		pbuf->AvgSum += d;

		pbuf->AvgPos = (pbuf->AvgPos + 1) % AVG_BUF_SIZE;

		pbuf->Avg = pbuf->AvgSum / AVG_BUF_SIZE;
	}
}

HP03M_Sensor Sens[MAX_SENS_COUNT];

void CalcPressTemp(void)
{
	float dUT;
	float OFF, SENS, X;
	float D2_C5, D2_C5_2;

	PHP03M_Sensor ps = &Sens[php];

	// Step 1: (get temperature value)
	D2_C5 = (s32) ps->H_D2 - (s32) ps->H_C5;
	D2_C5_2 = D2_C5 * D2_C5;
	if (D2_C5 > 0)
	{
		dUT = D2_C5 - (D2_C5_2 / 16384.0) * ps->H_A / (1L << ps->H_C);
	}
	else
	{
		dUT = D2_C5 - (D2_C5_2 / 16384.0) * ps->H_B / (1L << ps->H_C);
	}

	// Step 2: (calculate offset, sensitivity and final pressure value)
	OFF = (ps->H_C2 + (ps->H_C4 - 1024) * dUT / 16384) * 4;
	SENS = ps->H_C1 + ps->H_C3 * dUT / 1024;
	X = SENS * (ps->H_D1 - 7168) / 16384 - OFF;

	// �������� � �������� * 10 (����� ���� �����)
	float res = (X * 100 / 32 + ps->H_C7 * 10) * 10 + ps->H_Press_Offset; /// 1;
	if ((res >= 0) && (res <= 10000000.0)) //1300000.0))
	{
		ps->H_Press = res;

		AvgAdd(&ps->AvgPress, res);

//		ps->AvgPressBuf[ps->AvgPressPos] = res;
//		ps->AvgPressPos = (ps->AvgPressPos + 1) % AVG_BUF_SIZE;
//		ps->AvgPress = 0.0;
//		for (int i = 0; i < AVG_BUF_SIZE; i++)
//		{
//			ps->AvgPress += ps->AvgPressBuf[i];
//		}
//		ps->AvgPress /= AVG_BUF_SIZE;
	}
	else
	{
		ps->H_Press = 0;
	}
// For altitude measurement system, recommend to use P=X*100/2^5+C7*10
// So that better altitude resolution can be achieved

// Step 3: (calculate temperature)
//	ps->H_Temp = (250 + dUT * ps->H_C6 / 65536 - dUT / (1 << ps->H_D)) / 10;
	// ����������� � �������� * 10 (����� ���� �����)
	res = (250 + dUT * ps->H_C6 / 65536 - dUT / (1 << ps->H_D)) / 1 + ps->H_Temp_Offset;
	if ((res >= -200.0) && (res <= 600.0))
	{
		ps->H_Temp = res;

		AvgAdd(&ps->AvgTemp, res);
//		ps->AvgTempBuf[ps->AvgTempPos] = res;
//		ps->AvgTempPos = (ps->AvgTempPos + 1) % AVG_BUF_SIZE;
//		ps->AvgTemp = 0.0;
//		for (int i = 0; i < AVG_BUF_SIZE; i++)
//		{
//			ps->AvgTemp += ps->AvgTempBuf[i];
//		}
//		ps->AvgTemp /= AVG_BUF_SIZE;
	}
	else
	{
		ps->H_Temp = 0;
	}

//*
//	 float test;
//	 test = (ps->H_D2 - ps->H_C5);
//	printf("test   = %u\t%u\n", (u32) ps->H_D1, (u32) ps->H_D2);
//	 printf("D2_C5_x= %u\n", (u32)(ps->H_D2 - ps->H_C5));

//	 printf("D2_C5  = %u\n", (u32)D2_C5);
//	 printf("D2_C5_2= %u\n", (u32)D2_C5_2);
//	 printf("dUT    = %u\n", (u32)dUT);
//	 printf("OFF    = %u\n", (u32)OFF);
//	 printf("SENS   = %u\n", (u32)SENS);
//	 printf("X      = %u\n", (u32)X);
//	 printf("P      = %u\n", (u32)ps->H_Press);
//	 printf("T      = %u\n", u(u32)ps->H_Temp);
//*/
}

float I2CS_Receive_U16_as_float(float oldValue, u16 minValue, u16 maxValue)
{
	u16 msb, lsb, intRes;
	float res;

	msb = I2CS_ReceiveByte(SET);
	lsb = I2CS_ReceiveByte(SET);
	intRes = (msb * 256) + lsb;

	if ((intRes >= minValue) && (intRes <= maxValue))
	{
		res = intRes;
		return (res);
	}
	else
	{
		return oldValue;
	}
}

static bool HP03M_ReadCalibr(void);

void HP03M_Init(void)
{
	HP03MLowInit();

	I2CSoftInit();

	while (!HP03M_ReadCalibr())
		;
}

//void HP03M_ReadCalibr(void)
//{
//	GPIO_SetValue(OUT02_Port, _BV(OUT02_bit));
//
//	for (int p = 0; p < SensCount; p++)
//	{
//		php = p;
//		PHP03M_Sensor ps = &Sens[php];
//
//		XCLR_L();
//		delay(10);
//
//		u8 i = 16;
//		I2CS_Start();
//		I2CS_SendByte(0xA0);
//		I2CS_SendByte(i);
//		I2CS_Stop();
//
//		I2CS_Start();
//		I2CS_SendByte(0xA1);
//
//		ps->H_C1 = I2CS_Receive_U16_as_float();
//		ps->H_C2 = I2CS_Receive_U16_as_float();
//		ps->H_C3 = I2CS_Receive_U16_as_float();
//		ps->H_C4 = I2CS_Receive_U16_as_float();
//		ps->H_C5 = I2CS_Receive_U16_as_float();
//		ps->H_C6 = I2CS_Receive_U16_as_float();
//		ps->H_C7 = I2CS_Receive_U16_as_float();
//
//		ps->H_A = I2CS_ReceiveByte(SET);
//		ps->H_B = I2CS_ReceiveByte(SET);
//		ps->H_C = I2CS_ReceiveByte(SET);
//		ps->H_D = I2CS_ReceiveByte(RESET);
//
//		I2CS_Stop();
//
//		/*
//		 printf("C1 = %u\n", (u32) ps->H_C1);
//		 printf("C2 = %u\n", (u32) ps->H_C2);
//		 printf("C3 = %u\n", (u32) ps->H_C3);
//		 printf("C4 = %u\n", (u32) ps->H_C4);
//		 printf("C5 = %u\n", (u32) ps->H_C5);
//		 printf("C6 = %u\n", (u32) ps->H_C6);
//		 printf("C7 = %u\n", (u32) ps->H_C7);
//
//		 printf("A = %u\n", (u32) ps->H_A);
//		 printf("B = %u\n", (u32) ps->H_B);
//		 printf("C = %u\n", (u32) ps->H_C);
//		 printf("D = %u\n", (u32) ps->H_D);
//		 //*/
//	}
//	GPIO_ClearValue(OUT02_Port, _BV(OUT02_bit));
//}
//

static bool HP03M_ReadCalibr(void)
{
	typedef enum T_Stage
	{
		AInitLoop = 0,
		ABeginLoop,
		A1,
		A2,
		A3,
		A4,
		A5,
		A6,
		A7,
		A8,
		A9,
		A10,
		A11,
		A12,
		AEndLoop
	} TStage;

	u8 test(u8 newValue, u8 minValue, u8 maxValue, u8 oldValue)
	{
//		if ((newValue >= minValue) && (newValue <= maxValue))
//		{
		return newValue;
//		}
//		else
//		{
//			return oldValue;
//		}
	}

	static TStage stage = AInitLoop;
	static int p;
	static PHP03M_Sensor ps;
	bool res = false;

	switch (stage)
	{
		case AInitLoop:
			p = 0;
			Next()
;			case ABeginLoop:
			php = p;
			ps = &Sens[php];
			XCLR_L();
			LDelay(50);
			Next();

			case A1:
			I2CS_Start();
			I2CS_SendByte(0xA0);
			I2CS_SendByte(16); //u8 i = 16; I2CS_SendByte(i);
			I2CS_Stop();

			I2CS_Start();
			I2CS_SendByte(0xA1);

			Next();

			case A2:
			ps->H_C1 = I2CS_Receive_U16_as_float(ps->H_C1,256,65535);
			Next();

			case A3:
			ps->H_C2 = I2CS_Receive_U16_as_float(ps->H_C2,0,8191);
			Next();

			case A4:
			ps->H_C3= I2CS_Receive_U16_as_float(ps->H_C3,0,3000);
			Next();

			case A5:
			ps->H_C4 = I2CS_Receive_U16_as_float(ps->H_C4,0,4096);
			Next();

			case A6:
			ps->H_C5 = I2CS_Receive_U16_as_float(ps->H_C5,4096,65535);
			Next();

			case A7:
			ps->H_C6 = I2CS_Receive_U16_as_float(ps->H_C6,0,16384);
			Next();

			case A8:
			ps->H_C7 = I2CS_Receive_U16_as_float(ps->H_C7,2400,2600);
			Next();

			case A9:
			ps->H_A = test(I2CS_ReceiveByte(SET), 1, 63, ps->H_A);
			Next();

			case A10:
			ps->H_B = test(I2CS_ReceiveByte(SET), 1, 63, ps->H_B);
			Next();

			case A11:
			ps->H_C = test(I2CS_ReceiveByte(SET), 1, 15, ps->H_C);
			Next();

			case A12:
			ps->H_D = test(I2CS_ReceiveByte(RESET), 1, 15, ps->H_D);
			I2CS_Stop();
			Next();

			case AEndLoop:
			p++;
			if (p < SensCount)
			{
				Goto(ABeginLoop);
			}
			else
			{

//				for(int i = 0; i < SensCount; i++)
//				{
//					u32 c1 = Sens[i].H_C1,
//					c2 = Sens[i].H_C2,
//					c3 = Sens[i].H_C3,
//					c4 = Sens[i].H_C4,
//					c5= Sens[i].H_C5,
//					c6= Sens[i].H_C6,
//					c7= Sens[i].H_C7,
//					a = Sens[i].H_A,
//					b = Sens[i].H_B,
//					c = Sens[i].H_C,
//					d = Sens[i].H_D,
//					d1 = Sens[i].H_D1,
//					d2 = Sens[i].H_D2;
//
//					printf("CALIBR:(%u)\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n",
//							(i+1), c1, c2, c3, c4, c5, c6, c7, a, b ,c, d, d1, d2);
//				}

				Goto(AInitLoop);
				res = true;
			}
			break;
		}

	return res;
}

//static void HP03M_GetValues(void)
//{
//	for (int p = 0; p < SensCount; p++)
//	{
//		php = p;
//		PHP03M_Sensor ps = &Sens[php];
//
//		ps->H_D2 = HP03M_TemperatureADC();
//		ps->H_D1 = HP03M_PressureADC();
//
//		CalcPressTemp();
//	}
//}

static bool HP03M_GetValues(void)
{
	typedef enum T_HP03M_GetValues_Stage
	{
		HP_GV_Init = 0,
		HP_GV_TemperatureADC = 1,
		HP_GV_PressureADC = 2,
		HP_GV_TestEnd = 3
	} THP03MGetValuesStage;

	static THP03MGetValuesStage stage = HP_GV_Init;
	static int p;
	bool res = false;
	u16 adcres;

	switch (stage)
	{
		case HP_GV_Init:
			p = 0;

			Next()
;			case HP_GV_TemperatureADC:
			php = p;

			adcres = HP03M_TemperatureADC();

			if (adcres != (u16) -2) // -2 - ������� ���������������
			{
				// ���������, stage ��������� � ����������
				Sens[php].H_D2 = adcres;
				Next();
			}

			break;

			case HP_GV_PressureADC:
			adcres = HP03M_PressureADC();
			if (adcres != (u16) -2) // -2 - ������� ���������������
			{
				Sens[php].H_D1 = adcres;
				CalcPressTemp();

				Next();
			}
			break;

			case HP_GV_TestEnd:
			p++;
			if (p >= SensCount)
			{
				res = true;
				Goto(HP_GV_Init);
			}
			else
			{
				Goto(HP_GV_TemperatureADC);
			}
			break;
		}

	return res;

//	for (int p = 0; p < SensCount; p++)
//	{
//		php = p;
//		PHP03M_Sensor ps = &Sens[php];
//
//		ps->H_D2 = HP03M_TemperatureADC();
//		ps->H_D1 = HP03M_PressureADC();
//
//		CalcPressTemp();
//	}
}

u64 maxtime = 0;
u64 deltatime = 0;

void hp03m_process(void)
{
	typedef enum T_HP03M_Stage
	{
		HP_None = 0,
		HP_ReadCalibr,
		HP_GetValues,
	} THP03MStage;

	static THP03MStage stage = HP_None;

	static u32 get_values_refresh_period = 1;
	static u32 read_calibr_refresh_period = 10;
	static u32 get_values_rtc = 0;
	static u32 read_calibr_rtc = 0;

	u64 curtime = GetTicks() / 10;

	void PrintSens(void)
	{
		//*
		printf("%3u PRESS/TEMP: ", (u32)((GetTicks() / 10) % 1000));
		for (int p = 0; p < SensCount; p++)
		{
			u32 d1 = Sens[p].H_D1;
			u32 d2 = Sens[p].H_D2;
			u32 hp = Sens[p].H_Press;
			u32 ht = Sens[p].H_Temp;
			u32 ap = Sens[p].AvgPress.Avg;
			u32 at = Sens[p].AvgTemp.Avg;
			printf("\t(%X/%X)", d1, d2);
			printf("[%u.%u/%u.%u  %u.%u/%u.%u]", hp / 10, hp % 10, ht / 10, ht % 10, ap / 10, ap % 10, at / 10,
			at % 10);
		}
		CR();
	}

// ���-�� ��������
	if (IfInDelay())
		return;

	time_t cur_rtc;

	switch (stage)
	{
		case HP_None:
			cur_rtc = GetRTC();

			if (cur_rtc >= read_calibr_rtc)
			{
				read_calibr_rtc = cur_rtc + read_calibr_refresh_period;

				Goto(HP_ReadCalibr);
				break;
			}

			if (cur_rtc >= get_values_rtc)	// ��� � �������
			{
				get_values_rtc = cur_rtc + get_values_refresh_period;

				Goto(HP_GetValues);
				break;
			}

//			u32 one10sec = one1000sec / 10;
//			if (get_values_rtc != one10sec)	// 10 ��� � �������
//			{
//				get_values_rtc = one10sec;
//
//				Goto(HP_GetValues);
//				break;
//			}

			Goto(HP_GetValues);

			break;

		case HP_ReadCalibr:
			if (HP03M_ReadCalibr())
			{
				Goto(HP_None);
			}
			break;

		case HP_GetValues:
			if (HP03M_GetValues())
			{
//				PrintSens();
				Goto(HP_None);
			}
			break;
	}

	deltatime = GetTicks() / 10 - curtime;
	if (deltatime > maxtime)
	{
		maxtime = deltatime;
		printf("---------------------------------MAX HP03M TIME = %u\n", (u32) maxtime);
	}
}

