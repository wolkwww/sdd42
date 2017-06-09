// Relay.c

#include <stdio.h>

#include "types.h"

#include "common.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "Relay.h"

void Proc_ON(PObject obj, PVar var)
{
	SetVarValueByName(obj, "STATE", "1");
	FireEvent(obj, "ON_ON");
}
void Proc_OFF(PObject obj, PVar var)
{
	SetVarValueByName(obj, "STATE", "0");
	FireEvent(obj, "ON_OFF");
}
void Proc_INVERT(PObject obj, PVar var)
{
	u32 state = ValueAsInt(GetObjVarValue(obj, "STATE"));
	if (state != 0)
	{
		SetVarValueByName(obj, "STATE", "0");
		FireEvent(obj, "ON_OFF");
	}
	else
	{
		SetVarValueByName(obj, "STATE", "1");
		FireEvent(obj, "ON_ON");
	}
}


void Proc_STATE(PObject obj, PVar var)
{
	u32 state = ValueAsInt(GetObjVarValue(obj, "STATE"));
	if (state != 0)
	{
		FireEvent(obj, "ON_ON");
	}
	else
	{
		FireEvent(obj, "ON_OFF");
	}
}

const struct TVarDef VarDef[] =
{
  { "ON", Proc_ON, null, 3, "" },
  { "OFF", Proc_OFF, null, 3, "" },
  { "INVERT", Proc_INVERT, null, 3, "" },
  { "STATE", Proc_STATE, null, 3, "0" },
};

const struct TEventDef RelayEventDef[] =
{
{ "ON_ON" },
  { "ON_OFF" }, };

const struct TType TRelay =
{ "RELAY", // name
  null, // main proc
  null,	// InitProc

  null,

  (sizeof(VarDef) / sizeof(struct TVarDef)),
  VarDef,

  (sizeof(RelayEventDef) / sizeof(struct TEventDef)),
  RelayEventDef,

  0
};

