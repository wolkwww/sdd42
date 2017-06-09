// Comparer.c

#include "types.h"
#include <stdio.h>

#include "common.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "Comparer.h"

//struct TUserData
//{
//	u8 state_low;
//	u8 state_mid;
//	u8 state_high;
//};
//
//typedef struct TUserData* PUserData;
//
//#define ud  			((PUserData)(obj->UserData))

//static void Proc_SetVAL(PObject obj)
//{
//	u32 val = ValueAsInt(GetObjVarValue(obj, "VAL"));
//
////		printf("%u < %u < %u\n\r", val, ValueAsInt(GetObjVarValue(obj, "MIN")), ValueAsInt(GetObjVarValue(obj, "MAX")));
//
//	if (val <= ValueAsInt(GetObjVarValue(obj, "MIN")))
//	{
//		FireEvent(obj, "ON_LOW");
//		return;
//	}
//
//	if (val >= ValueAsInt(GetObjVarValue(obj, "MAX")))
//	{
//		FireEvent(obj, "ON_HIGH");
//		return;
//	}
//
//	FireEvent(obj, "ON_MID");
//	return;
//}

static void Proc_SetVAL(PObject obj, PVar var)
{
	float val = ValueAsFloat(GetObjVarValue(obj, "VAL"));

//		printf("%u < %u < %u\n\r", val, ValueAsInt(GetObjVarValue(obj, "MIN")), ValueAsInt(GetObjVarValue(obj, "MAX")));

	if (val <= ValueAsFloat(GetObjVarValue(obj, "MIN")))
	{
		FireEvent(obj, "ON_LOW");
	}
	else
	{
		if (val >= ValueAsFloat(GetObjVarValue(obj, "MAX")))
		{
			FireEvent(obj, "ON_HIGH");
		}
		else
		{
			FireEvent(obj, "ON_MID");
		}
	}
}

//static void MainProc(PObject obj)
//{
//	if (obj->InputsChanged)
//	{
//		u32 val = ValueAsInt(GetObjVarValue(obj, "VAL"));
//
////		printf("%u < %u < %u\n\r", val, ValueAsInt(GetObjVarValue(obj, "MIN")), ValueAsInt(GetObjVarValue(obj, "MAX")));
//
//		if (val <= ValueAsInt(GetObjVarValue(obj, "MIN")))
//		{
//			FireEvent(obj, "ON_LOW");
//			return;
//		}
//
//		if (val >= ValueAsInt(GetObjVarValue(obj, "MAX")))
//		{
//			FireEvent(obj, "ON_HIGH");
//			return;
//		}
//
//		FireEvent(obj, "ON_MID");
//		return;
//	}
//}

static const struct TVarDef VarDef[] =
{
{ "VAL", Proc_SetVAL, null, 10, "1" },
  { "MAX", null, null, 10, "2" },
  { "MIN", null, null, 10, "0" }, };

static const struct TEventDef _EventDef[] =
{
{ "ON_LOW", 0 },
  { "ON_MID", 0 },
  { "ON_HIGH", 0 } };

const struct TType TComparer =
{ "COMPARER", null, //MainProc,
  null,	// InitProc

  null,

  (sizeof(VarDef) / sizeof(struct TVarDef)),
  VarDef,

  (sizeof(_EventDef) / sizeof(struct TEventDef)),
  _EventDef,

  0	//sizeof(struct TUserData)
};

