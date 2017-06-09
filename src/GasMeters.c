//GasMeters.c
/*
 * GasMeters.c
 *
 *  Created on: 26.03.2013
 *      Author: mrhru
 */

#include "defines.h"
#ifdef GAZ_STATION

#include <stdio.h>

#include "LPC17xx.h"
#include "core_cm3.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

#include "common.h"
#include "GasMeters.h"
#include "HAL.h"
#include "UnitTypes.h"
#include "Utils.h"

// это не работает, прерывани€ GPIO только дл€ портов 0 и 2
//#define METER_1KHZ_Port				(1)
//#define METER_1KHZ_Pin				(19)
// используем один из зарезервированных IO - IO01
#define METER_1KHZ_Port				(0)
#define METER_1KHZ_Pin				(24)

#define METER_LITER_1_Port			(0)
#define METER_LITER_1_Pin			(4)

#define METER_LITER_2_Port			(0)
#define METER_LITER_2_Pin			(5)

#define METER_LITER_3_Port			(0)
#define METER_LITER_3_Pin			(17)

#define METER_LITER_4_Port			(0)
#define METER_LITER_4_Pin			(18)

#define METER_CHECK_1_Port			(0)
#define METER_CHECK_1_Pin			(19)

#define METER_CHECK_2_Port			(0)
#define METER_CHECK_2_Pin			(20)

#define METER_CHECK_3_Port			(0)
#define METER_CHECK_3_Pin			(21)

#define METER_CHECK_4_Port			(0)
#define METER_CHECK_4_Pin			(22)

#define METER_TEST_Port				(1)
#define METER_TEST_Pin				(20)

//u8 MeterPorts[METER_COUNT] =
//{ METER_1_Port, METER_2_Port, METER_3_Port, METER_4_Port };
//u8 MeterPins[METER_COUNT] =
//{ METER_1_Pin, METER_2_Pin, METER_3_Pin, METER_4_Pin };

TMeterPort MeterPorts[METER_COUNT] =
{
{ METER_LITER_1_Port, METER_LITER_1_Pin, METER_CHECK_1_Port, METER_CHECK_1_Pin },
  { METER_LITER_2_Port, METER_LITER_2_Pin, METER_CHECK_2_Port, METER_CHECK_2_Pin },
  { METER_LITER_3_Port, METER_LITER_3_Pin, METER_CHECK_3_Port, METER_CHECK_3_Pin },
  { METER_LITER_4_Port, METER_LITER_4_Pin, METER_CHECK_4_Port, METER_CHECK_4_Pin }, };

volatile u32 OneKHzCounter = 0;

TMeterStruct Meters[4];

void PerformMeter(int n);
void StopMeter(int n, bool error);

void EINT3_IRQHandler(void)
{
//	LED1_ON();

	if (LPC_GPIOINT ->IO0IntStatF == _BV(METER_1KHZ_Pin))
	{
		LPC_GPIOINT ->IO0IntClr = _BV(METER_1KHZ_Pin);
		OneKHzCounter++;

		GPIO_ClearValue(METER_TEST_Port, _BV(METER_TEST_Pin));

//		if ((OneKHzCounter & 0x01) == 0)
//		{
//			LED2_OFF();
//		}
//		else
//		{
//			LED2_ON();
//		}

		for (int i = 0; i < METER_COUNT; i++)
		{
			if (Meters[i].started)
			{
//				LED2_ON();
				PerformMeter(i);
//				LED2_OFF();
			}
		}
	}
//	LED1_OFF();
}

bool ReadMeterState(int n)
{
	// выходы счетчиков с открытым коллектором, поэтому переход в 0 - это начало импульса
	return GPIO_Read(MeterPorts[n].LiterPort, _BV(MeterPorts[n].LiterPin)) == 0 ? true : false;
}

void PerformMeter(int n)
{
	PMeter pm = &Meters[n];

	pm->timeout_counter++;

	if (pm->liter_counter >= 0)
	{
		pm->time_counter++;
	}

	if (pm->timeout_counter > pm->time_max)
	{
		StopMeter(n, true);
		return;
	}

	pm->liter_state = ReadMeterState(n); // определ€ем состо€ние входа

	if (!pm->liter_prev_state)
	{
		if (pm->liter_state)
		{
			GPIO_SetValue(METER_TEST_Port, _BV(METER_TEST_Pin));
			// обнаружили импульс
			pm->liter_counter++;

//			// обнулили таймаут
			pm->timeout_counter = 0;	//??? - заработало //- это вредно, в Proc_Main из-за этого генер€тс€ ошибки

			if (pm->time_counter > pm->time_min)
			{
				// врем€ больше минимального, это последний литр
				StopMeter(n, false);
				return;
			}
		}
	}

	pm->liter_prev_state = pm->liter_state;
}

void StartMeter(int n)
{
	EnableDebugUart = false;

	PMeter pm = &Meters[n];

	pm->done = false;
	pm->error = false;

	pm->todo_done = false;
	pm->todo_error = false;

	pm->time_counter = 0;
	pm->timeout_counter = 0;
	pm->liter_counter = -1;
	pm->liter_prev_state = pm->liter_state = ReadMeterState(n);

	pm->rtc_prev = pm->oneKHzCounter_prev = GetRTC();
	pm->started = true; // устанавливаем последним
}

void StopMeter(int n, bool error)
{
	PMeter pm = &Meters[n];

	pm->started = false; // устанавливаем первым

	pm->done = pm->todo_done = error ? false : true;
	pm->error = pm->todo_error = error;

	EnableDebugUart = true;
}

void Meters_Init(void)
{
	void InitGPIO_IN(u8 port, u8 bit)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 0;
		PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL; /**< Pin is in the normal (not open drain) mode.*/
		PinCfg.Pinmode = PINSEL_PINMODE_PULLUP; //PINSEL_PINMODE_TRISTATE; //
		PinCfg.Portnum = port;
		PinCfg.Pinnum = bit;
		PINSEL_ConfigPin(&PinCfg);
		GPIO_SetDir(port, _BV(bit), INPUT);
	}

	void InitGPIO_OUT(u8 port, u8 bit)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 0;
		PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL; //PINSEL_PINMODE_OPENDRAIN;
		PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
		PinCfg.Portnum = port;
		PinCfg.Pinnum = bit;
		PINSEL_ConfigPin(&PinCfg);
		GPIO_SetDir(port, _BV(bit), OUTPUT);
	}

	//TODO
	msgn("Meters_Init");

	InitGPIO_IN(METER_1KHZ_Port, METER_1KHZ_Pin);

	// Ћитры
	for (int i = 0; i < METER_COUNT; i++)
	{
		InitGPIO_IN(MeterPorts[i].LiterPort, MeterPorts[i].LiterPin);
	}

	// —игнал CHECK ENABLE
	for (int i = 0; i < METER_COUNT; i++)
	{
		InitGPIO_OUT(MeterPorts[i].CheckPort, MeterPorts[i].CheckPin);
		GPIO_ClearValue(MeterPorts[i].CheckPort, _BV(MeterPorts[i].CheckPin));
	}

	InitGPIO_OUT(METER_TEST_Port, METER_TEST_Pin);
	GPIO_ClearValue(METER_TEST_Port, _BV(METER_TEST_Pin));

	//Initialize EXT registers
	LPC_SC ->EXTINT = 0x0;
	LPC_SC ->EXTMODE = 0x0;
	LPC_SC ->EXTPOLAR = 0x0;

	LPC_SC ->EXTMODE = 0x01; // edge sensitive, only EINT0
	LPC_SC ->EXTPOLAR = 0x0; // falling-edge sensitive
	LPC_SC ->EXTINT = 0xF; // External Interrupt Flag cleared

	// Enable GPIO interrupt  METER_1KHZ_Pin
	LPC_GPIOINT ->IO0IntEnF = _BV(METER_1KHZ_Pin);

	NVIC_EnableIRQ(EINT3_IRQn);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// METER TYPE
static void Proc_Start(PObject obj, PVar var)
{
	EnableDebugUart = false;

	int n = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));
	printf("METER (%u) START\n\r", n);

	PMeter pm = &Meters[n];

	pm->time_min = ValueAsInt(GetObjVarValue(obj, "TIME_MIN")) * 1000;
	pm->time_max = ValueAsInt(GetObjVarValue(obj, "TIME_MAX")) * 1000;

	StartMeter(n);

	FireEvent(obj, "ON_START");
}

static void Proc_Stop(PObject obj, PVar var)
{
	int n = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));
	StopMeter(n, false);
}

static void Proc_CheckEn(PObject obj, PVar var)
{
	int n = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));
	int check_en = ValueAsInt(GetObjVarValue(obj, "CHECK_EN"));

	printf("CHECK_EN Meter%u = %u\n\r", n, check_en);

	if (check_en != 0)
	{
		GPIO_ClearValue(MeterPorts[n].CheckPort, _BV(MeterPorts[n].CheckPin));
		printf("CLEAR\n\r");
	}
	else
	{
		GPIO_SetValue(MeterPorts[n].CheckPort, _BV(MeterPorts[n].CheckPin));
		printf("SET\n\r");
	}
}

static void Proc_MAIN(PObject obj)
{
	int n = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));

	PMeter pm = &Meters[n];

	if (pm->started)
	{
		if (pm->rtc_prev != GetRTC()) // изменились секунды
		{
			pm->rtc_prev = GetRTC();
			if (pm->oneKHzCounter_prev == OneKHzCounter)
			{
				// но состо€ние общего времени измерени€ не изменилось
				// следовательно, нет сигнала от генератора 1к√ц
				StopMeter(n, true);
			}
			pm->oneKHzCounter_prev = OneKHzCounter;
		}
	}

	PVar var;
	var = FindVar(obj, "TIME");
	SetVarAsInt(var, pm->time_counter);
	var = FindVar(obj, "LITERS");
	SetVarAsInt(var, pm->liter_counter);
	var = FindVar(obj, "STARTED");
	SetVarAsInt(var, pm->started);
	var = FindVar(obj, "DONE");
	SetVarAsInt(var, pm->done);

	var = FindVar(obj, "ERROR");
	SetVarAsInt(var, pm->error);

	if (pm->todo_done == true)
	{
		pm->todo_done = false;
		FireEvent(obj, "ON_DONE");
	}

	if (pm->todo_error == true)
	{
		pm->todo_error = false;
		FireEvent(obj, "ON_ERROR");
	}

	u8 buf[32];
	int res = (pm->done == true) ? 1 : (pm->error == true) ? 2 : 0;
	sprintf(buf, "%u,%u,%u", res, pm->time_counter, pm->liter_counter);
	SetVarValueByName(obj, "RESULT", buf);
}

const struct TVarDef MeterVarDef[] =
{
{ "START", Proc_Start, null, 2, "0" }, // команда на запуск
  { "STOP", Proc_Stop, null, 2, "0" }, // команда на запуск

  { "TIME", null, null, 10, "0" }, // врем€ между первым и последним импульсом
  { "LITERS", null, null, 8, "0" }, // счетчик литровых импульсов, не счита€ стартового

  // 0,1234567890,1234567890
  { "RESULT", null, null, 32, "0" }, // результы счета через зап€тую [0|1|2],[TIME],[LITERS]

  { "STARTED", null, null, 2, "0" }, // идет измерение
  { "DONE", null, null, 2, "0" }, // завершено
  { "ERROR", null, null, 2, "0" }, // ошибка

  { "TIME_MIN", null, null, 8, "300" }, // врем€, после которого ожидаетс€ последний литровый импульс и закачиваетс€ измерение
  { "TIME_MAX", null, null, 8, "500" }, // таймаут, после которого измерени€ заканчиваютс€

  { "CHECK_EN", Proc_CheckEn, null, 2, "0" }, // уровень на счетчик, включающий режим поверки(активный ноль)

  { "CHANNEL", null, null, 2, "0" } // номер счетчика, 0..3
};

const struct TEventDef MeterEventDef[] =
{
{ "ON_START", 0 },
  { "ON_DONE", 0 },
  { "ON_ERROR", 0 } };

const struct TType TMeter =
{ "METER", // name
  Proc_MAIN, // main proc
  null,	// InitProc

  null,

  (sizeof(MeterVarDef) / sizeof(struct TVarDef)),
  MeterVarDef,

  (sizeof(MeterEventDef) / sizeof(struct TEventDef)),
  MeterEventDef, 0 };

#endif
