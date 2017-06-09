/*
 * TADC.c
 *
 *  Created on: 28 янв. 2016 г.
 *      Author: mrhru
 */

#include <stdio.h>
#include <math.h>

#include "LPC17xx.h"
#include "core_cm3.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

#include "common.h"
#include "HAL.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "CPU.h"

#include "TADC.h"
#include "adc.h"

typedef struct
{
	double adc_min;
	double adc_max;

	double value_min;
	double value_max;

	int channel;

	double min_delta;
	double lastval;

	PVar var_value;
	PVar var_adc;

	double average;
	int aveN;
	u64 last_update_time;
	u64 filter_interval;
} TADCUserDataDef;

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static void Proc_MAIN(PObject obj)
{
	TADCUserDataDef* pdata = (TADCUserDataDef*) obj->UserData;

	if ((pdata->channel == -1) && (pdata->var_value->CPUPinNumber != -1))
	{
		printf("pdata->var_value->CPUPinNumber: %d\r\n", pdata->var_value->CPUPinNumber);

		PCPUPin pin = &CPUPins[pdata->var_value->CPUPinNumber];
		if (pin != NULL)
		{
			pdata->channel = pin->PinNumber;
			printf("SET ADC <%s> CHANNEL to %d\r\n", obj->Name, pdata->channel);
		}
	}

//	printf("%s %d\r\n", obj->Name, pdata->channel);

	if ((pdata->channel >= 0) && (pdata->channel < ADC_COUNT))
	{
		if (pdata->adc_max == pdata->adc_min)
		{
			pdata->adc_min = pdata->value_min;
			pdata->adc_max = pdata->value_max;

			if (pdata->adc_max == pdata->adc_min)
			{
				pdata->adc_min = pdata->value_min = 0.0;
				pdata->adc_max = pdata->value_max = 1.0;
			}
		}

		s32 raw_adc = adc_values[pdata->channel];
		double val = (raw_adc - pdata->adc_min) / (pdata->adc_max - pdata->adc_min) * (pdata->value_max - pdata->value_min) + pdata->value_min;

		pdata->average += val;
		pdata->aveN ++;

		u64 curr_time = GetTicksMs();
		if (curr_time > (pdata->last_update_time + pdata->filter_interval))
		{
			val = pdata->average / pdata->aveN;

//			printf("ADC: %d\t%f\r\n", pdata->aveN, val);

			pdata->average = 0.0;
			pdata->aveN = 0;
			pdata->last_update_time = curr_time;

			sprintf(pdata->var_value->Value, "%f", val);
			sprintf(pdata->var_adc->Value, "%d", raw_adc);

			if (fabs(pdata->lastval - val) >= pdata->min_delta)
			{
				FireEvent(obj, "ON_CHANGE");
				pdata->lastval = val;

//				printf("ADC%d:\t%f\r\n", pdata->channel, val);
			}
		}
	}
}

static void Proc_UpdateParams(PObject obj, PVar var)
{
	TADCUserDataDef* pdata = (TADCUserDataDef*) obj->UserData;

	pdata->var_value = FindVar(obj, "VALUE");
	pdata->var_adc = FindVar(obj, "ADC");

	pdata->adc_min = ValueAsFloat(GetObjVarValue(obj, "ADC_MIN"));
	pdata->adc_max = ValueAsFloat(GetObjVarValue(obj, "ADC_MAX"));

	pdata->value_min = ValueAsFloat(GetObjVarValue(obj, "VALUE_MIN"));
	pdata->value_max = ValueAsFloat(GetObjVarValue(obj, "VALUE_MAX"));

	pdata->min_delta = ValueAsFloat(GetObjVarValue(obj, "MIN_DELTA"));

	pdata->average = 0.0;
	pdata->aveN = 0;
	pdata->last_update_time = 0;
	pdata->filter_interval = ValueAsInt(GetObjVarValue(obj, "FILTER_MS"));

	pdata->channel = -1;

	printf("ADC\tINIT pdata\r\n");
}

static void Proc_Init(PObject obj)
{
	Proc_UpdateParams(obj, NULL);
}

const struct TVarDef ADCVarDef[] =
{
{ "ADC", null, null, 16, "1" },	// raw adc data
    { "ADC_MIN", Proc_UpdateParams, null, 16, "0" },
    { "ADC_MAX", Proc_UpdateParams, null, 16, "1" },

    { "VALUE", null, null, 16, "0" },	//
    { "VALUE_MIN", Proc_UpdateParams, null, 16, "0" },
    { "VALUE_MAX", Proc_UpdateParams, null, 16, "1" },

    { "MIN_DELTA", Proc_UpdateParams, null, 16, "1" },
    { "FILTER_MS", Proc_UpdateParams, null, 16, "100" }, };

const struct TEventDef ADCEventDef[] =
{
{ "ON_CHANGE", 0 } };

const struct TType TADCSensor =
{ "ADC", // name
    Proc_MAIN, // main proc
    Proc_Init, // InitProc

    null, // ObjCmds;		// команды для объектов

    (sizeof(ADCVarDef) / sizeof(struct TVarDef)),
    ADCVarDef,

    (sizeof(ADCEventDef) / sizeof(struct TEventDef)),
    ADCEventDef,

    sizeof(TADCUserDataDef) //	UserDataSize
    };

