/*
 * Multiplier.c
 *
 *  Created on: 25.06.2014
 *      Author: mrhru
 */

#include "types.h"
#include <stdio.h>

#include "common.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "ABCalc.h"

// ����������

static void Proc_SetAB(PObject obj, PVar var)
{
	double A = ValueAsFloat(GetObjVarValue(obj, "A"));
	double B = ValueAsFloat(GetObjVarValue(obj, "B"));

	SetVarValueAsDouble(obj, "A+B", A + B);
	SetVarValueAsDouble(obj, "A-B", A - B);
	SetVarValueAsDouble(obj, "A*B", A * B);
	SetVarValueAsDouble(obj, "A/B", A / B);

	FireEvent(obj, "ON_DONE");
}


static const struct TVarDef VarDef[] =
{
{ "A", Proc_SetAB, null, 16, "0" },
  { "B", Proc_SetAB, null, 16, "0" },
  { "A+B", null, null, 16, "0" },
  { "A-B", null, null, 16, "0" },
  { "A*B", null, null, 16, "0" },
  { "A/B", null, null, 16, "0" }, };

static const struct TEventDef EventDef[] =
{
{ "ON_DONE", 0 } };

const struct TType TABCalc =
{ "ABCALC", null,	// main proc
  null,	// InitProc

  null,	// commands

  (sizeof(VarDef) / sizeof(struct TVarDef)), VarDef,

  (sizeof(EventDef) / sizeof(struct TEventDef)),
  EventDef,

  0 };

