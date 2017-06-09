/*
 * Dictionary.c
 *
 *  Created on: 12 февр. 2016 г.
 *      Author: mrhru
 */

#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "Transport.h"
#include "Dictionary.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define N_INPUT	"INPUT"
#define N_CLEAR	"CLEAR"

#define N_V0	"V0"
#define N_V1	"V1"
#define N_V2	"V2"
#define N_V3	"V3"
#define N_V4	"V4"
#define N_V5	"V5"
#define N_V6	"V6"
#define N_V7	"V7"
#define N_V8	"V8"
#define N_V9	"V9"
#define N_ON_V0	"ON_V0"
#define N_ON_V1	"ON_V1"
#define N_ON_V2	"ON_V2"
#define N_ON_V3	"ON_V3"
#define N_ON_V4	"ON_V4"
#define N_ON_V5	"ON_V5"
#define N_ON_V6	"ON_V6"
#define N_ON_V7	"ON_V7"
#define N_ON_V8	"ON_V8"
#define N_ON_V9	"ON_V9"

#define MAX_ENTRY_COUNT	10

static const char* N_Vs[MAX_ENTRY_COUNT] =
{ N_V0, N_V1, N_V2, N_V3, N_V4, N_V5, N_V6, N_V7, N_V8, N_V9 };

static const char* N_Evs[MAX_ENTRY_COUNT] =
{ N_ON_V0, N_ON_V1, N_ON_V2, N_ON_V3, N_ON_V4, N_ON_V5, N_ON_V6, N_ON_V7, N_ON_V8, N_ON_V9 };

typedef struct
{
	char* Names[MAX_ENTRY_COUNT];
	PVar Vars[MAX_ENTRY_COUNT];
} TDictUserDataDef;

static void Proc_CLEAR(PObject obj, PVar var)
{
	TDictUserDataDef* pdata = (TDictUserDataDef*) obj->UserData;

	for (int i = 0; i < MAX_ENTRY_COUNT; i++)
	{
		if (pdata->Names[i] != null)
		{
			free(pdata->Names[i]);
			pdata->Names[i] = null;
		}
	}
}

static void Proc_INPUT(PObject obj, PVar var)
{
	TDictUserDataDef* pdata = (TDictUserDataDef*) obj->UserData;

	char* msg = GetObjVarValue(obj, N_INPUT);

	// input string in format "NAME=VALUE"

	char* name_end = strchr(msg, '=');
	if ((name_end != NULL) && (name_end != msg))
	{
		bool found = false;
		int pos = 0;
		for (; pos < MAX_ENTRY_COUNT; pos++)
		{
			if (pdata->Names[pos] == null)
			{
				// end of names
				break;
			}
			if (CompareName(msg, name_end, pdata->Names[pos]))
			{
				// entry found, update value
				SetVarValue(pdata->Vars[pos], name_end + 1);
				FireEvent(obj, N_Evs[pos]);
				found = true;
				break;
			}
		}

		if (!found)
		{
			msgn("DICT: entry not found, make new");
			if (pos <= MAX_ENTRY_COUNT)
			{
				// insert new entry
				pdata->Names[pos] = makestrbyend(msg, name_end);
				SetVarValue(pdata->Vars[pos], name_end + 1);
				FireEvent(obj, N_Evs[pos]);
			}
			else
			{
				msgn("DICT: not enough room");
			}
		}
	}
	else
	{
		printf("DICT: illegal format <%s>\r\n", msg);
	}
}

static void Proc_INIT(PObject obj)
{
	TDictUserDataDef* pdata = (TDictUserDataDef*) obj->UserData;

	for (int i = 0; i < MAX_ENTRY_COUNT; i++)
	{
		pdata->Vars[i] = FindVar(obj, N_Vs[i]);
	}
}

static const struct TVarDef VarDef[] =
{ //
{ N_INPUT, Proc_INPUT, null, 32, "" },
{ N_CLEAR, Proc_CLEAR, null, 0, "" },
{ N_V0, null, null, 16, "0" },
{ N_V1, null, null, 16, "0" },
{ N_V2, null, null, 16, "0" },
{ N_V3, null, null, 16, "0" },
{ N_V4, null, null, 16, "0" },
{ N_V5, null, null, 16, "0" },
{ N_V6, null, null, 16, "0" },
{ N_V7, null, null, 16, "0" },
{ N_V8, null, null, 16, "0" },
{ N_V9, null, null, 16, "0" },

};

static const struct TEventDef EventDef[] =
{
{ N_ON_V0, 0 },
{ N_ON_V1, 0 },
{ N_ON_V2, 0 },
{ N_ON_V3, 0 },
{ N_ON_V4, 0 },
{ N_ON_V5, 0 },
{ N_ON_V6, 0 },
{ N_ON_V7, 0 },
{ N_ON_V8, 0 },
{ N_ON_V9, 0 },
};

const struct TType TDictionary =
{ "DICT",		// NAME
  null,			// main proc
  Proc_INIT,	// InitProc

  null,		// commands

  (sizeof(VarDef) / sizeof(struct TVarDef)), VarDef,

  (sizeof(EventDef) / sizeof(struct TEventDef)),
  EventDef,

  sizeof(TDictUserDataDef) //	UserDataSize
};

