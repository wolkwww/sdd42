// RoundRobin4.c
/*
 * RoundRobin4.c
 *
 *  Created on: 17.04.2013
 *      Author: mrhru
 */

#include <stdio.h>

#include "types.h"

#include "common.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "RoundRobin4.h"

//командный автомат
// ѕри согнале START - вырабатываетс€ первое событие ON_1
// ѕри последующих сигналах NEXT - следующие событи€
// ≈сли ON_4 уже сработало, то выдаетс€ ON_DONE и свойство STATE устанавливаетс€ в 0
//
// ¬арианты использовани€
//START	-> ON_1
//NEXT	-> ON_2
//NEXT	-> ON_3
//NEXT	-> ON_4
//NEXT	-> ON_DONE
//START	-> ON_1
//NEXT	-> ON_2
// ...
//
// ...
//NEXT	-> ON_3
//NEXT	-> ON_4
//NEXT	-> ON_DONE
//START	-> ON_1
//NEXT	-> ON_2
//NEXT	-> ON_3
//NEXT	-> ON_4
//NEXT	-> ON_DONE
//START	-> ON_1
// ...
//
//START ->	ON_1 (state = 1)
//
//NEXT  ->	state: 0..3
//				state = state + 1
//				ON_[state]
//			state: 4
//				state = 0
//				ON_DONE

void Proc_Start(PObject obj, PVar var)
{
	SetVarValueByName(obj, "STATE", "1");
	FireEvent(obj, "ON_1");
	msgn("ROUND_ROBIN: ON_1");
}

void Proc_Next(PObject obj, PVar var)
{
	u32 state = ValueAsInt(GetObjVarValue(obj, "STATE"));

	switch (state)
	{
		case 0:
			SetVarValueByName(obj, "STATE", "1");
			FireEvent(obj, "ON_1");
			msgn("ROUND_ROBIN: ON_1");
			break;
		case 1:
			SetVarValueByName(obj, "STATE", "2");
			FireEvent(obj, "ON_2");
			msgn("ROUND_ROBIN: ON_2");
			break;
		case 2:
			SetVarValueByName(obj, "STATE", "3");
			FireEvent(obj, "ON_3");
			msgn("ROUND_ROBIN: ON_3");
			break;
		case 3:
			SetVarValueByName(obj, "STATE", "4");
			FireEvent(obj, "ON_4");
			msgn("ROUND_ROBIN: ON_4");
			break;
		case 4:
			SetVarValueByName(obj, "STATE", "0");
			FireEvent(obj, "ON_DONE");
			msgn("ROUND_ROBIN: ON_DONE");
			break;
	}
}

static const struct TVarDef VarDef[] =
{
{ "START", Proc_Start, null, 0, null },
  { "NEXT", Proc_Next, null, 0, null },
  { "STATE", null, null, 10, "0" }, };

static const struct TEventDef EventDef[] =
{
{ "ON_1", 0 },
  { "ON_2", 0 },
  { "ON_3", 0 },
  { "ON_4", 0 },
  { "ON_DONE", 0 } };

const struct TType TRR4 =
{ "RR4",	// name
  null,		// main proc
  null,		// InitProc

  null,	// commands

  (sizeof(VarDef) / sizeof(struct TVarDef)), VarDef,

  (sizeof(EventDef) / sizeof(struct TEventDef)),
  EventDef,

  0 };

