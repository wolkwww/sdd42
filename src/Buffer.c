// Buffer.c
/*
 *  Created on: 19.02.2013
 *      Author: mrhru
 */

#include "types.h"

#include <stdio.h>

#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"

#include "Buffer.h"

static void Proc_Input(PObject obj, PVar var)
{
	FireEvent(obj, "OUT");
}

const struct TVarDef BufferVarDef[] =
{
	{"IN", 		Proc_Input, null, 16,	""},
};

const struct TEventDef BufferEventDef[] =
{
	{"OUT",	0}
};

const struct TType TBuffer =
{
	"BUFFER",
	null, 	// Proc_MAIN,
	null,	// InitProc

	null,

	(sizeof(BufferVarDef) / sizeof(struct TVarDef)),
	BufferVarDef,

	(sizeof(BufferEventDef) / sizeof(struct TEventDef)),
	BufferEventDef
};

