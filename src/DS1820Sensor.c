/*
 * DS1820Sensor.c
 *
 *  Created on: 12.09.2014
 *      Author: mrhru
 */

#include "defines.h"
#ifdef TERMO_STATION

#include <stdio.h>

#include "types.h"

#include "UnitTypes.h"
#include "Utils.h"
#include "SimpleSens.h"
#include "Objects.h"
#include "HP03M.h"

#include "string.h"

// SIMPLE SENSOR TYPE
//static void Proc_SetChannel(PObject obj)
//{
//	int n = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));
//	printf("METER (%u) START\n\r", n);
//
////	PMeter pm = &Meters[n];
////
////	pm->time_min = ValueAsInt(GetObjVarValue(obj, "TIME_MIN")) * 1000;
////	pm->time_max = ValueAsInt(GetObjVarValue(obj, "TIME_MAX")) * 1000;
////
////	StartMeter(n);
////
////	FireEvent(obj, "ON_START");
//}

const struct TVarDef DS1820SensorVarDef[] =
{
{ "VALUE", null, null, 16, "1" },
//{ "CHANNEL", Proc_SetChannel, null, 16, "0" },
  { "OFFSET", null, null, 16, "0" }, };


const struct TType TDS1820Sensor =
{ "DS1820",	// name
  null,		// main proc
  null,		// InitProc

  null,

  (sizeof(DS1820SensorVarDef) / sizeof(struct TVarDef)),
  DS1820SensorVarDef,

  0, //(sizeof(SimpleSensEventDef) / sizeof(struct TEventDef)),
  NULL , //SimpleSensEventDef
};

#endif
