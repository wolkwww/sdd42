//Timer.c

#include <stdio.h>

#include "types.h"

#include "UnitTypes.h"
#include "Utils.h"
#include "Timer.h"
#include "HAL.h"

// TIMER TYPE
static void Proc_MAIN(PObject obj)
{
	PVar var;
	var = FindVar(obj, "STATE");
	SetVarAsInt(var, (*((u32*) obj->UserData) != 0) ? 1 : 0);

	if ((*((u32*) obj->UserData) != 0) && (GetTicks() / 10 > *((u32*) obj->UserData)))
	{
		*((u32*) obj->UserData) = 0;
		FireEvent(obj, "ON_TIME");
	}
}

static void Proc_START(PObject obj, PVar var)
{
	u32 period = ValueAsInt(GetObjVarValue(obj, "PERIOD"));
	// запоминаем текущее время + уст. период
	*((u32*) obj->UserData) = GetTicks() / 10 + period;
}
static void Proc_STOP(PObject obj, PVar var)
{
	*((u32*) obj->UserData) = 0;
}

const struct TVarDef TimerVarDef[] =
{
{ "START", Proc_START, null, 0 },
  { "STOP", Proc_STOP, null, 0 },
  { "PERIOD", null, null, 10, "1000" },
  { "STATE", null, null, 2 }, };

const struct TEventDef TimerEventDef[] =
{
{ "ON_TIME" }, };

const struct TType TTimer =
{ "TIMER",		// name
  Proc_MAIN,	// main proc
  null,			// InitProc

  null,

  (sizeof(TimerVarDef) / sizeof(struct TVarDef)),
  TimerVarDef,

  (sizeof(TimerEventDef) / sizeof(struct TEventDef)),
  TimerEventDef,

  sizeof(u32)			// место под таймер
};

