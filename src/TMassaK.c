/*
 * TMassaK.c
 *
 *  Created on: 4 мая 2016 г.
 *      Author: mrhru
 */

#include "defines.h"
#ifdef WATER_STATION

#include <stdio.h>
#include <math.h>

#include "LPC17xx.h"
#include "core_cm3.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

#include "common.h"
#include "HAL.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "CPU.h"
#include "UARTS.h"

#include "TMassaK.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define N_CHANNEL 		"CHANNEL"

#define N_RESET			"RESET"
#define N_GET			"GET"
#define N_WEIGHT		"WEIGHT"
#define N_DONE			"DONE"
#define N_ON_DONE		"ON_DONE"


#define pdata		((TUserDataDef*) obj->UserData)

typedef struct
{
	int channel;
	int pos;
	u8 buf[10];
	u64 lastReceivedTime;
} TUserDataDef;

// 4800, 11 бит = 436 б/с
// таймаут = 1/436 = 2,3 мс

#define RX_TIMEOUT		60	// in tens of ms

static void Proc_MAIN(PObject obj)
{
//	TUserDataDef* pdata = (TUserDataDef*) obj->UserData;

	if (pdata->channel != -1)
	{
		while (std_get_available(pdata->channel))
		{
			u64 currReceivedTime = GetTicks();
			u64 deltaRT = currReceivedTime - pdata->lastReceivedTime;
			if (deltaRT > RX_TIMEOUT)
			{
				pdata->pos = 0;
			}

			pdata->lastReceivedTime = currReceivedTime;

			u8 c = std_get(pdata->channel);
			pdata->buf[pdata->pos] = c;
//			printf("0x%2X [%d] %LU\r\n", c, pdata->pos, deltaRT);

			pdata->pos ++;

			if (pdata->pos >= 5)
			{
				bool done = (pdata->buf[0] & 0x80) != 0; 		//D7 - индикатор процесса взвешивания 0-не завершен, 1-завершен

				bool oneTens = pdata->buf[1] != 0; 				//D8-D15 - дискретность отсчета, если 0x01 - в десятых грамма

				bool negative = (pdata->buf[4] & 0x80) != 0;	//D39 - знак

				s32 weight = pdata->buf[4] & 0x7F;
				weight = weight * 256 + pdata->buf[3];
				weight = weight * 256 + pdata->buf[2];

				if (negative)
				{
					weight = -weight;
				}

				if (oneTens)
				{
					weight = weight / 10;
				}

				SetVarValueAsInt(obj, N_WEIGHT, weight);
				SetVarValueAsInt(obj, N_DONE, done ? 1 : 0);

				FireEvent(obj, N_ON_DONE);
				printf("WEIGHT := <%d>\r\n", weight);
			}

			break;
		}
	}
}

static void Proc_SetChannel(PObject obj, PVar var)
{
//	TUserDataDef* pdata = (TUserDataDef*) obj->UserData;

	pdata->channel = ValueAsInt(FindVar(obj, N_CHANNEL)->Value);
	if (pdata->channel != -1)
	{
		pdata->pos = 0;

		_STD_UART_Init(pdata->channel, 4800, UART_PARITY_EVEN);

		printf("LIBRA at ch[%d], baud: %d\r\n", pdata->channel, 4800);
	}
}

static void _sendToLibra(PObject obj, char* s, char* comment)
{
//	TUserDataDef* pdata = (TUserDataDef*) obj->UserData;

	if (pdata->channel != -1)
	{
//		msgn(comment);
		std_msg(pdata->channel, s);
	}
}

static void Proc_SetGET(PObject obj, PVar var)
{
	char s[2] =
	{ 0x4A, 0x00 };

	_sendToLibra(obj, s, "LIBRA CALL");
}

static void Proc_SetRESET(PObject obj, PVar var)
{
	char s[2] =
	{ 0x0D, 0x00 };

	_sendToLibra(obj, s, "LIBRA RESET");
}


static void Proc_INIT(PObject obj)
{
//	TUserDataDef* pdata = (TUserDataDef*) obj->UserData;
	pdata->pos = 0;
	pdata->lastReceivedTime = 0;
}

static const struct TVarDef VarDef[] =
{
{ N_RESET, Proc_SetRESET, null, 0, "" },
{ N_GET, Proc_SetGET, null, 0, "" },

{ N_CHANNEL, Proc_SetChannel, null, 2, "-1" },

{ N_WEIGHT, null, null, 15, "0" },
{ N_DONE, null, null, 15, "0" },

};

static const struct TEventDef EventDef[] =
{
{ N_ON_DONE, 0 }, //
    };

const struct TType TMassaK =
{ "MASSAK", // name
    Proc_MAIN, // main proc
    Proc_INIT, // InitProc

    null, // ObjCmds;		// команды для объектов

    (sizeof(VarDef) / sizeof(struct TVarDef)),
    VarDef,

    (sizeof(EventDef) / sizeof(struct TEventDef)),
    EventDef,

    sizeof(TUserDataDef) //	UserDataSize
    };

#endif
