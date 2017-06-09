/*
 * TRS232.c
 *
 *  Created on: 5 февр. 2016 г.
 *      Author: mrhru
 */

#include <stdio.h>
#include <string.h>
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

#include "TRS232.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define N_RX			"RECEIVE"
#define N_TX			"SEND"
#define N_CHANNEL 		"CHANNEL"
#define N_BAUD 			"BAUD"
#define N_WEIGHT		"WEIGHT"

#define N_ON_RECEIVE	"ON_RECEIVE"
#define N_ON_WEIGHT		"ON_WEIGHT"

typedef struct
{
	int channel;
	int pos;
	u8* buf;
} TRS232UserDataDef;

static void Proc_MAIN(PObject obj)
{
	TRS232UserDataDef* pdata = (TRS232UserDataDef*) obj->UserData;

	if (pdata->channel != -1)
	{
		PVar rxvar = FindVar(obj, N_RX);
		u16 size = rxvar->VarDef->ValueSize - 1;

		while (std_get_available(pdata->channel))
		{
			u8 c = std_get(pdata->channel);

			if ((c == '\0') || (c == '\r') || (c == '\n') || (pdata->pos >= size))
			{
//				rxvar->Value[pdata->pos] = '\0';
				pdata->buf[pdata->pos] = '\0';

				if (pdata->pos > 0)
				{
					strncpy(rxvar->Value, pdata->buf, size);
					FireEvent(obj, N_ON_RECEIVE);
					printf("RS232 <- <%s>\r\n", rxvar->Value);

					//ParseAs_Massa_K_Libra()
					if ((rxvar->Value[0] == 0x55) && (rxvar->Value[1] == 0xAA))
					{
						int weight = rxvar->Value[2] + (rxvar->Value[3] * 256);

						if(rxvar->Value[4] == 0x80)
						{
							weight = -weight;
						}
						SetVarValueAsInt(obj, N_WEIGHT, weight);

						FireEvent(obj, N_ON_WEIGHT);
						printf("WEIGHT := <%d>\r\n", -weight);
					}
				}

				pdata->pos = 0;

				break;
			}
			else
			{
//				rxvar->Value[pdata->pos] = c;
				pdata->buf[pdata->pos] = c;

				pdata->pos ++;
			}
		}
	}
}

static void Proc_SetChannel(PObject obj, PVar var)
{
	TRS232UserDataDef* pdata = (TRS232UserDataDef*) obj->UserData;

	pdata->channel = ValueAsInt(FindVar(obj, N_CHANNEL)->Value);
	pdata->pos = 0;

	printf("RS232 ch: %d\r\n", pdata->channel);
}

static void Proc_SetBaud(PObject obj, PVar var)
{
	TRS232UserDataDef* pdata = (TRS232UserDataDef*) obj->UserData;

	if (pdata->channel != -1)
	{
		u32 baud = ValueAsInt(FindVar(obj, N_BAUD)->Value);
//		std_set_baud(pdata->channel, baud);

		_STD_UART_Init(pdata->channel, baud, UART_PARITY_NONE);

		printf("RS232 ch[%d], baud: %d\r\n", pdata->channel, baud);
	}
}

static void Proc_Send(PObject obj, PVar var)
{
	TRS232UserDataDef* pdata = (TRS232UserDataDef*) obj->UserData;

	if (pdata->channel != -1)
	{
		char* msg = GetObjVarValue(obj, N_TX);

		printf("RS232 SEND <%s>\r\n", msg);

		std_msg(pdata->channel, msg);
		std_msg(pdata->channel, "\r\n");
	}
}

static void Proc_INIT(PObject obj)
{
	PVar rxvar = FindVar(obj, N_RX);

	TRS232UserDataDef* pdata = (TRS232UserDataDef*) obj->UserData;
	pdata->buf = getmem(rxvar->VarDef->ValueSize + 1, "TRS232UserDataDef->buf");
}

const struct TVarDef RS232VarDef[] =
{
{ N_RX, null, null, 255, "" },
{ N_TX, Proc_Send, null, 255, "" },
{ N_CHANNEL, Proc_SetChannel, null, 2, "-1" },
{ N_BAUD, Proc_SetBaud, null, 8, "115200" },
{ N_WEIGHT, null, null, 15, "0" }, };

const struct TEventDef RS232EventDef[] =
{
{ N_ON_RECEIVE, 0 }, //
    { N_ON_WEIGHT, 0 }, //
    };

const struct TType TRS232 =
{ "RS232", // name
    Proc_MAIN, // main proc
    Proc_INIT, // InitProc

    null, // ObjCmds;		// команды для объектов

    (sizeof(RS232VarDef) / sizeof(struct TVarDef)),
    RS232VarDef,

    (sizeof(RS232EventDef) / sizeof(struct TEventDef)),
    RS232EventDef,

    sizeof(TRS232UserDataDef) //	UserDataSize
    };

