//MainModule.c

#include "types.h"
#include <stdio.h>

#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "Common.h"
#include "HAL.h"
#include "S42Common.h"
#include "Transport.h"
#include "MainModule.h"
#include "transport_low.h"
#include "config.h"

static void Proc_Main(PObject obj)
{
	// TODO
}

static void Proc_ListObjects(PObject obj, PVar var)
{
	msgn("Call LIST");
	PObject subobj = obj->ObjList;

	if (subobj != null )
	{
		int cnt = 1;
		printf("\t\t\t\t\t\t\tLIST:\t(%d)\t%s\n", cnt++, subobj->Name);
		while (subobj->next != null )
		{
			subobj = subobj->next;
			printf("\t\t\t\t\t\t\tLIST:\t(%d)\t%s\n", cnt++, subobj->Name);
		}
	}
}

static void Proc_Create(PObject obj, PVar var)
{
	char* msg = GetObjVarValue(obj, "CREATE");
	PObject newobj = CreateObject2(msg);

	if (newobj != null )
	{
		PObject subobj = obj->ObjList;

		if (subobj != null )
		{
//			printf("\t\t\t\t\t\t\tLIST:\t%s\n", subobj->Name);
			while (subobj->next != null )
			{
				subobj = subobj->next;
//				printf("\t\t\t\t\t\t\tLIST:\t%s\n", subobj->Name);
			}
			subobj->next = newobj;
		}
		else
		{
			obj->ObjList = newobj;
		}
//		printf("\t\t\t\t\t\t\tLIST:\t%s\n", newobj->Name);

		printf("\t\t\t\tCreated Object <%s>at ADDR\t|%d\n", newobj->Name, (u32) newobj);
	}
}

static void Proc_Reset(PObject obj, PVar var)
{
	msgn("MAIN.RESET");
	delay(1000);
	Reset();
}

int push_cmd_cnt = 0;

static void Proc_END_PUSH_CMD(PObject obj, PVar var)
{
	printf("push_cmd_cnt = %u\n", push_cmd_cnt);
	push_cmd_cnt = 0;
}


static void Proc_Config(PObject obj, PVar var)
{
	// TODO
	char* msg = GetObjVarValue(obj, "CONFIG");
	// cfgStr начинается с разделяющей запятой, не забывать пропускать
	char* cfgStr = FindEndOfAddrItem(msg, ',');

	printf("Proc_Config\n");

	if (CompareName(msg, cfgStr, "CLEAR"))
	{
		Config_ClearAll();
		msgn("MAIN.CONFIG.CLEAR");
		Config_TestShow();
	}
	else if (CompareName(msg, cfgStr, "ADD"))
	{
		cfgStr++; // пропуск разделяющей запятой
		Config_AddLine(cfgStr);
		printf("MAIN.CONFIG.ADD:%s\n", cfgStr);
	}
	else if (CompareName(msg, cfgStr, "SAVE"))
	{
		cfgStr++; // пропуск разделяющей запятой
		Config_Save();
		msgn("MAIN.CONFIG.SAVE");
		Config_TestShow();
	}
	else if (CompareName(msg, cfgStr, "GET"))
	{
		//TODO
		printf("MAIN.CONFIG.GET:%s\n", cfgStr);
		//Config_GetLine(u16 cnt);
	}
	else
	{
		printf("MAIN.CONFIG: UNKNOWN COMMAND <%s>\n", cfgStr);
	}
}

static void ConfigClearCmd(PObject obj, PVar var, char* param)
{
	msgn("MAIN.CONFIG.CLEAR");
	Config_ClearAll();
	Config_TestShow();
}
static void ConfigAddCmd(PObject obj, PVar var, char* param)
{
	printf("MAIN.CONFIG.ADD:%s\n", param);
	Config_AddLine(param);
}
static void ConfigSaveCmd(PObject obj, PVar var, char* param)
{
	msgn("MAIN.CONFIG.SAVE");
	Config_Save();
	Config_TestShow();
}
static void ConfigGetCmd(PObject obj, PVar var, char* param)
{
	//TODO
	printf("MAIN.CONFIG.GET:%s\n", param);
	//Config_GetLine(u16 cnt);
}

static void Proc_SetDate(PObject obj, PVar var)
{
	msg("SET DATE: ");
	msgn(var->Value);
}

static void Proc_SetTime(PObject obj, PVar var)
{
	msg("SET TIME: ");
	msgn(var->Value);
}


static const TVarCmdDef ConfigCmdDef[] =
{
{ "GET", ConfigGetCmd },		//
  { "CLEAR", ConfigClearCmd },
  { "ADD", ConfigAddCmd },
  { "SAVE", ConfigSaveCmd },

  { null, null }			// признак конца
};

static const struct TVarDef VarDef[] =
{
{ "CREATE", Proc_Create, null, 128, "" },
  { "RESET", Proc_Reset, null, 0, "" },
  { "END_PUSH_CMD", Proc_END_PUSH_CMD, null, 0, "" },
  { "CONFIG", Proc_Config, ConfigCmdDef, 128, "" },
  { "LIST", Proc_ListObjects, null, 0, "" },
  { "DATE", Proc_SetDate, null, 16, "" },
  { "TIME", Proc_SetTime, null, 16, "" },
};

static const struct TEventDef EventDef[] =
{
{ "ON_BOOT", 0 }, };

const struct TType TMain =
{ "MAIN", Proc_Main, null,	// InitProc

  null,

  (sizeof(VarDef) / sizeof(struct TVarDef)),
  VarDef,

  (sizeof(EventDef) / sizeof(struct TEventDef)),
  EventDef };

typedef struct TMain* PMain;

PObject MainModule;

