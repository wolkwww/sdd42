//SimpleSens.c

#include <stdio.h>

#include "types.h"

#include "UnitTypes.h"
#include "Utils.h"
#include "SimpleSens.h"
#include "Objects.h"
#include "HP03M.h"

#include "string.h"

// SIMPLE SENSOR TYPE

const struct TVarDef SimpleSensVarDef[] =
{
{ "STATE", null, null, 16, "1" },
  { "OFFSET", null, null, 16, "0" }, };

//const struct TEventDef SimpleSensEventDef[] =
//{
//	{"ON_ERROR",	0},
//};

const struct TType TSimpleSens =
{ "SENSOR",	// name
  null,		// main proc
  null,		// InitProc

  null,

  (sizeof(SimpleSensVarDef) / sizeof(struct TVarDef)),
  SimpleSensVarDef,

  0, //(sizeof(SimpleSensEventDef) / sizeof(struct TEventDef)),
  NULL , //SimpleSensEventDef
};

