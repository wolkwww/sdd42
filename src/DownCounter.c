/*
 * DownCounter.c
 *
 *  Created on: 18.04.2013
 *      Author: mrhru
 */

#include "types.h"
#include <stdio.h>

#include "common.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "DownCounter.h"

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

void Proc_Count(PObject obj, PVar var)
{
	s32 state = ValueAsInt(GetObjVarValue(obj, "STATE"));

	if (state > 0)
	{
		state--;
		SetVarValueAsInt(obj, "STATE", state);

		if (state == 0)
		{
			FireEvent(obj, "ON_DONE");
			msgn("DOWN_COUNTER: ON_DONE");
		}
	}
}

static const struct TVarDef VarDef[] =
{
{ "STATE", null, null, 10, "0" },
  { "COUNT", Proc_Count, null, 0, null }, };

static const struct TEventDef EventDef[] =
{
{ "ON_DONE", 0 } };

const struct TType TDownCounter =
{ "DOWN_COUNTER", null,	// main proc
  null,	// InitProc

  null,	// commands

  (sizeof(VarDef) / sizeof(struct TVarDef)), VarDef,

  (sizeof(EventDef) / sizeof(struct TEventDef)),
  EventDef,

  0 };

