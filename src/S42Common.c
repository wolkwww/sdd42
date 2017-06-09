// S42common.c

#include <stdio.h>

#include "defines.h"

#include "types.h"

#include "UnitTypes.h"
#include "Common.h"
#include "S42Common.h"

#include "MainModule.h"
#include "NetObject.h"
#include "Relay.h"
#include "Timer.h"
#include "Button.h"
#include "Comparer.h"
#include "SimpleSens.h"
#include "Buffer.h"

#include "RoundRobin4.h"
#include "DownCounter.h"
#include "ABCalc.h"
#include "TADC.h"
#include "TRS232.h"
#include "TScan.h"
#include "Dictionary.h"

#ifdef GAZ_STATION
#include "GasMeters.h"
#include "MeterProg.h"
#endif

#ifdef WATER_STATION
#include "WaterMeter.h"
#include "HBridge.h"
#include "TMassaK.h"
#endif

#ifdef TERMO_STATION
#include "DS1820Sensor.h"
#include "DS18B20Tray.h"
#endif


const PType Types[] = 
{
	&TMain,
	&TNet,
	&TRelay,
	&TTimer, 
	&TButton,
	&TComparer,
	&TSimpleSens,
	&TBuffer,
	&TRR4,
	&TDownCounter,
	&TABCalc,
//	&TADCSensor,
	&TScan,
	&TDictionary,
	&TRS232,


#ifdef GAZ_STATION
	&TMeter,
	&TMeterProg,
#endif

#ifdef WATER_STATION
	&TWaterMeter,
	&THBridge,
	&TMassaK,
#endif

#ifdef TERMO_STATION
	&TDS1820Sensor,
	&TTray,
#endif

	null
};



