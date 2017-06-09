
#include "defines.h"
#ifdef WATER_STATION

#include <stdio.h>

#include "types.h"

#include "common.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "HBridge.h"

#define N_ON1   	"ON1"
#define N_ON2   	"ON2"
#define N_OFF   	"OFF"
#define N__OUT1  	"_OUT1"
#define N__OUT2  	"_OUT2"
#define N_ON_ON1	"ON_ON1"
#define N_ON_ON2	"ON_ON2"
#define N_ON_OFF 	"ON_OFF"
#define N_HBRIDGE	"HBRIDGE"

static void Proc_ON1(PObject obj, PVar var)
{
	SetVarValueAsInt(obj, N_ON1, 1);
	SetVarValueAsInt(obj, N_ON2, 0);

	SetVarValueAsInt(obj, N__OUT2, 0);
	SetVarValueAsInt(obj, N__OUT1, 1);
	FireEvent(obj, N_ON_ON1);
	msgn("BRIDGE ON1");
}

static void Proc_ON2(PObject obj, PVar var)
{
	SetVarValueAsInt(obj, N_ON1, 0);
	SetVarValueAsInt(obj, N_ON2, 1);

	SetVarValueAsInt(obj, N__OUT1, 0);
	SetVarValueAsInt(obj, N__OUT2, 1);
	FireEvent(obj, N_ON_ON2);
	msgn("BRIDGE ON2");
}

static void Proc_OFF(PObject obj, PVar var)
{
	SetVarValueAsInt(obj, N__OUT1, 0);
	SetVarValueAsInt(obj, N__OUT2, 0);
	FireEvent(obj, N_ON_OFF);
	msgn("BRIDGE OFF");
}

static const struct TVarDef VarDef[] =
{
  { N_ON1, Proc_ON1, null, 3, "0" },
  { N_ON2, Proc_ON2, null, 3, "0" },
  { N_OFF, Proc_OFF, null, 0, "" },
  { N__OUT1, null, null, 3, "0" },
  { N__OUT2, null, null, 3, "0" },
};

static const struct TEventDef EventDef[] =
{
	{ N_ON_ON1 },
	{ N_ON_ON2 },
  	{ N_ON_OFF },
};

const struct TType THBridge =
{ N_HBRIDGE, // name
  null, // main proc
  null,	// InitProc

  null,

  (sizeof(VarDef) / sizeof(struct TVarDef)),
  VarDef,

  (sizeof(EventDef) / sizeof(struct TEventDef)),
  EventDef,

  0
};

#endif
