//Button.cpp

#include "types.h"

#include <stdio.h>

#include "common.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "Button.h"
#include "Objects.h"

struct TButtonUserData
{
	int			OldState;
};
typedef	struct TButtonUserData* PButtonUserData;

// BUTTON TYPE
static void Proc_MAIN(PObject obj)
{	
	int oldstate = ((PButtonUserData)obj->UserData)->OldState;
	int state = ValueAsInt(GetObjVarValue(obj, "STATE"));
	
	if (  oldstate != state)
	{
		((PButtonUserData)obj->UserData)->OldState = state;
		if (state != 0)
		{
			msgn("ON_ON");
			FireEvent(obj, "ON_ON");
		}
		else
		{
			msgn("ON_OFF");
			FireEvent(obj, "ON_OFF");
		}
		FireEvent(obj, "ON_CHANGE");
	}

}

const struct TVarDef ButtonVarDef[] =
{
	{"STATE", 		null, null, 2,	"1"}	// размер = 2, потому как строка
};

const struct TEventDef ButtonEventDef[] =
{
	{"ON_ON",		0},
	{"ON_OFF",		0},
	{"ON_CHANGE",	0}
};

const struct TType TButton =
{
	"BUTTON",
	Proc_MAIN,
	null,	// InitProc
	
	null,

	(sizeof(ButtonVarDef) / sizeof(struct TVarDef)),
	ButtonVarDef,
	
	(sizeof(ButtonEventDef) / sizeof(struct TEventDef)),
	ButtonEventDef,

	sizeof(struct TButtonUserData),
};

