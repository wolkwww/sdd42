/*
 * MeterProg.c
 *
 *  Created on: 06.11.2013
 *      Author: mrhru
 */

#include "defines.h"
#ifdef GAZ_STATION

#include <stdio.h>
#include <stdbool.h>

#include "LPC17xx.h"
#include "core_cm3.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

#include "common.h"
#include "MeterProg.h"
#include "HAL.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "UARTS.h"
#include "LowTimers.h"

#define ANSWER_TIME_OUT				100

typedef enum
{
	MeterProgAnswer_None,
	MeterProgAnswer_OK,
	MeterProgAnswer_Error
} MeterProgAnswer;

u8 OKAnswers[METER_COUNT] =
{ 0, 0, 0, 0 };
u8 ErrorAnswers[METER_COUNT] =
{ 0, 0, 0, 0 };

LowTimer AnswerTimers[METER_COUNT] =
{ 0, 0, 0, 0 };

void ClearAnswer(int ch)
{
	OKAnswers[ch] = 0;
	ErrorAnswers[ch] = 0;
	std_get_clear(ch);
	InitTimer(&AnswerTimers[ch]);
}

MeterProgAnswer CheckAnswer(u8 ch)
{
	if (std_get_available(ch))
	{
		u8 res = std_get(ch);

		if (res == '1')
		{
			// если пришел правильный символ, сбрасываем таймер
			InitTimer(&AnswerTimers[ch]);

			OKAnswers[ch]++;
			ErrorAnswers[ch] = 0;
			if (OKAnswers[ch] == 3)
			{
				printf("ANSWER:\tOK\n\r");
				return MeterProgAnswer_OK;
			}
		}
		else
		{
			if (res == '0')
			{
				// если пришел правильный символ, сбрасываем таймер
				InitTimer(&AnswerTimers[ch]);

				ErrorAnswers[ch]++;
				OKAnswers[ch] = 0;
				if (ErrorAnswers[ch] == 3)
				{
					printf("ANSWER:\tERROR\n\r");
					return MeterProgAnswer_Error;
				}
			}
		}
	}
	else
	{
		u64 ms = GetTimerMs(AnswerTimers[ch]);
		if (ms > ANSWER_TIME_OUT)
		{
			printf("ANSWER:\tTIMEOUT (%u)\n\r", (u32) ms);
			return MeterProgAnswer_Error;
		}
	}
	return MeterProgAnswer_None;
}

bool ProgMeter(int ch, u8 cmd, u8 addr, u8 data)
{
	MeterProgAnswer res;

	bool test_ok(void)
	{
		while ((res = CheckAnswer(ch)) == MeterProgAnswer_None)
			;
		return res == MeterProgAnswer_OK;
	}

	printf("\nCH:\t%u\tCMD: '%c'\tADDR:\t%u\tDATA:\t%u (0x%x)\n\r", ch, cmd, (int) addr, (int) data, (int) data);

	ClearAnswer(ch);

	int DELAY = 3;

	_std_sendchar(ch, 'B');			// prefix
	delay(DELAY);
	_std_sendchar(ch, cmd);			// write cmd
	delay(DELAY);
	_std_sendchar(ch, addr);		// addr
	delay(DELAY);
	_std_sendchar(ch, data);		//data
	delay(DELAY);
	_std_sendchar(ch, 'E');			// postfix
	delay(DELAY);
	_std_sendchar(ch, 'B' + cmd + addr + data + 'E');	//Check Sum

	return test_ok();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
// METER_PROG TYPE

static void Proc_PROGTABLE(PObject obj, PVar var)
{
	EnableDebugUart = false;

	PVar varDone = FindVar(obj, "DONE");
	SetVarAsInt(varDone, 0);

	PVar varError = FindVar(obj, "ERROR");
	SetVarAsInt(varError, 0);

	bool res = false;

	int ch = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));

	if ((ch >= 0) && (ch < METER_COUNT))
	{
		// запись
		char* buf = GetObjVarValue(obj, "PROGTABLE");
		printf("METER(%u) PROG TABLE = %s\n\r", ch, buf);

		u32 N; // номер элемента таблицы 0..15
		u32 F, A, B, neg;
		if (sscanf(buf, "%u,%u,%u,%u,%u", &N, &F, &A, &B, &neg) == EOF)
		{
			EnableDebugUart = true;
			msgn("ERROR in input string");
			SetVarAsInt(varError, 1);
			return;
		}
		printf("%u\t%u\t%u\t%u\t%u\n\r", N, F, A, B, neg);
		// N     F     A          b neg
		//12,12345,65535,4000000000,1

		u8 addr = N * (2 + 2 + 4 + 1) + 1; // + 1 это пропуск байта-кол-ва элементов

		res = true;

		res = res && ProgMeter(ch, 'W', addr + 0, (u8) (F >> 8));
		res = res && ProgMeter(ch, 'W', addr + 1, (u8) (F));

		res = res && ProgMeter(ch, 'W', addr + 2, (u8) (A >> 8));
		res = res && ProgMeter(ch, 'W', addr + 3, (u8) (A));

		res = res && ProgMeter(ch, 'W', addr + 4, (u8) (B >> 24));
		res = res && ProgMeter(ch, 'W', addr + 5, (u8) (B >> 16));
		res = res && ProgMeter(ch, 'W', addr + 6, (u8) (B >> 8));
		res = res && ProgMeter(ch, 'W', addr + 7, (u8) (B));

		res = res && ProgMeter(ch, 'W', addr + 8, (u8) (neg));
	}

	if (res)
	{
		printf("METER(%u) PROG TABLE = OK\n\r", ch);
		SetVarAsInt(varDone, 1);
	}
	else
	{
		printf("METER(%u) PROG TABLE = ERROR\n\r", ch);
		SetVarAsInt(varError, 1);
	}
	EnableDebugUart = true;
}

// программирование серийного номера и даты
static void Proc_PROGID(PObject obj, PVar var)
{
	EnableDebugUart = false;

	PVar varDone = FindVar(obj, "DONE");
	SetVarAsInt(varDone, 0);

	PVar varError = FindVar(obj, "ERROR");
	SetVarAsInt(varError, 0);

	int ch = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));

	bool res1 = FALSE, res2  = FALSE, res3 = FALSE, res4 = FALSE, res5 = FALSE, res6 = FALSE, res7 = FALSE, res8 = FALSE, res9 = FALSE, resA = FALSE, resB = FALSE, resC = FALSE, resD = FALSE;

	if ((ch >= 0) && (ch < METER_COUNT))
	{
		// запись ID
		u32 id = ValueAsInt(GetObjVarValue(obj, "PROGID"));
		printf("METER(%u) PROG ID = %u/0x%x\n\r", ch, id, id);

		// запись даты
		tm tmr;
		GetCurrDateTime(&tmr);

		res1 = ProgMeter(ch, 'I', 0, (u8) (id >> 24));
		res2 = ProgMeter(ch, 'I', 1, (u8) (id >> 16));
		res3 = ProgMeter(ch, 'I', 2, (u8) (id >> 8));
		res4 = ProgMeter(ch, 'I', 3, (u8) (id >> 0));


//		if (GetCurrDateTime(&tmr))
//		{
		resB = ProgMeter(ch, 'C', 0, tmr.tm_year - 100);
		resC = ProgMeter(ch, 'C', 1, tmr.tm_mon + 1);
		resD = ProgMeter(ch, 'C', 2, tmr.tm_mday);

		res5 = ProgMeter(ch, 'D', 0, tmr.tm_year - 100);
		res6 = ProgMeter(ch, 'D', 1, tmr.tm_mon + 1);
		res7 = ProgMeter(ch, 'D', 2, tmr.tm_mday);

		res8 = ProgMeter(ch, 'T', 0, tmr.tm_hour);
		res9 = ProgMeter(ch, 'T', 1, tmr.tm_min);
		resA = ProgMeter(ch, 'T', 2, tmr.tm_sec);

//		};
	}

	if (res1 && res2 && res3 && res4 && res5 && res6 && res7 && res8 && res9 && resA && resB && resC && resD)
	{
		SetVarAsInt(varDone, 1);
	}
	else
	{
		SetVarAsInt(varError, 1);
	}
	EnableDebugUart = true;
}

static void Proc_START(PObject obj, PVar var)
{
	EnableDebugUart = false;

	PVar varDone = FindVar(obj, "DONE");
	SetVarAsInt(varDone, 0);

	PVar varError = FindVar(obj, "ERROR");
	SetVarAsInt(varError, 0);

	bool res = false;

	int ch = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));

	if ((ch >= 0) && (ch < METER_COUNT))
	{
		u16 prog = ValueAsInt(GetObjVarValue(obj, "START"));

		u8 addr = prog >> 8;
		u8 data = (u8) prog;

		res = ProgMeter(ch, 'W', addr, data);
	}

	if (res)
	{
		SetVarAsInt(varDone, 1);
	}
	else
	{
		SetVarAsInt(varError, 1);
	}
	EnableDebugUart = true;
}

static void Proc_MAIN(PObject obj)
{
//	int started = ValueAsInt(GetObjVarValue(obj, "STARTED"));
//	if (started != 0)
//	{
//		int ch = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));
//
//		int res = CheckAnswer(ch);
//
//		PVar varDone = FindVar(obj, "DONE");
//		PVar varError = FindVar(obj, "ERROR");
//		PVar varStarted = FindVar(obj, "STARTED");
//
//		switch (res)
//		{
//			case MeterProgAnswer_None:
//				return;
//
//			case MeterProgAnswer_OK:
//				SetVarAsInt(varDone, 1);
//				SetVarAsInt(varError, 0);
//				SetVarAsInt(varStarted, 0);
//				break;
//
//			case MeterProgAnswer_Error:
//				SetVarAsInt(varDone, 0);
//				SetVarAsInt(varError, 1);
//				SetVarAsInt(varStarted, 0);
//				break;
//		}
//	}
}

const struct TVarDef MeterProgVarDef[] =
{
{ "START", Proc_START, null, 7, "0" }, // программирование побайтово
  { "DONE", null, null, 2, "0" }, // завершено
  { "ERROR", null, null, 2, "0" }, // ошибка
  { "CHANNEL", null, null, 2, "0" }, // номер счетчика, 0..3
  { "PROGID", Proc_PROGID, null, 12, "0" }, // команда на запись сер. номера в счетчик (u32)
  { "PROGTABLE", Proc_PROGTABLE, null, 32, "0" }, // запись элемента таблицы, строка типа 18,1183,43314,1423268,1
};

const struct TEventDef MeterProgEventDef[] =
{
{ "ON_DONE", 0 },
  { "ON_ERROR", 0 } };

const struct TType TMeterProg =
{ "METERPROG", // name
  Proc_MAIN, 	// main proc
  null,			// InitProc

  null,

  (sizeof(MeterProgVarDef) / sizeof(struct TVarDef)),
  MeterProgVarDef,

  (sizeof(MeterProgEventDef) / sizeof(struct TEventDef)),
  MeterProgEventDef, 0 };

#endif
