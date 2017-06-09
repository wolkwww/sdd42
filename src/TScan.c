/*
 * TScan.c
 *
 *  Created on: 10 февр. 2016 г.
 *      Author: mrhru
 */

#include <stdio.h>
#include <string.h>

#include "LPC17xx.h"
#include "core_cm3.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

#include "common.h"
#include "HAL.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "CPU.h"

#include "TScan.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define N_INPUT 	"INPUT"
#define N_FORMAT	"FORMAT"
#define N_VALUE 	"VALUE"
#define N_ON_SCAN	"ON_SCAN"

static void Proc_Input(PObject obj, PVar var)
{
	char* format = GetObjVarValue(obj, N_FORMAT);
	if (strlen(format) > 0)
	{
		char* input = GetObjVarValue(obj, N_INPUT);
		if (strlen(input) > 0)
		{
			float val = 0.0;
			sscanf(input, format, &val);
			SetVarValueAsDouble(obj, N_VALUE, val);

			FireEvent(obj, N_ON_SCAN);

			printf("SCAN: <%f>\r\n", val);
		}
	}
}

const struct TVarDef ScanVarDef[] =
{
{ N_INPUT, Proc_Input, null, 127, "" },
  { N_FORMAT, null, null, 127, "" },
  { N_VALUE, null, null, 16, "" }, };

const struct TEventDef ScanEventDef[] =
{
{ N_ON_SCAN, 0 } };

const struct TType TScan =
{ "SCAN", // name
  null, // main proc
  null, // InitProc

  null, // ObjCmds;		// команды для объектов

  (sizeof(ScanVarDef) / sizeof(struct TVarDef)), ScanVarDef,

  (sizeof(ScanEventDef) / sizeof(struct TEventDef)),
  ScanEventDef,

  0 //	UserDataSize
};

