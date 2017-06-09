//CPU.c

#include <string.h>
#include <stdio.h>

#include "defines.h"

#include "types.h"
#include "lpc17xx_gpio.h"

#include "HAL.h"
#include "HP03M.h"
#include "SerialParallelPort.h"
#include "DS18B20.h"
#include "DS18B20Tray.h"

#include "CPU.h"

void CPUPin_DefaultSet(PCPUPin pin, s32 val);
void CPUPin_DefaultSetInv(PCPUPin pin, s32 val);
s32 CPUPin_DefaultGet(PCPUPin pin);
//s32 CPUPin_DefaultGetInv(PCPUPin pin);
void CPUPin_DefaultGetInv(PCPUPin pin, char* buf, u8 bufsize);

#ifdef GAZ_STATION
void CPUPin_GetTemperature(PCPUPin pin, char* buf, u8 bufsize);
void CPUPin_GetPressure(PCPUPin pin, char* buf, u8 bufsize);
void CPUPin_SetOffsetTemperature(PCPUPin pin, s32 val);
void CPUPin_SetOffsetPressure(PCPUPin pin, s32 val);
#endif

#ifdef TERMO_STATION
void CPUPin_GetDS18B20Temperature(PCPUPin pin, char* buf, u8 bufsize);
#endif



void CPUPin_SerParSet(PCPUPin pin, s32 val);

//s32 CPUPin_Meter(PCPUPin pin);
void CPUPin_Meter(PCPUPin pin, char* buf, u8 bufsize);

const struct TCPUPin CPUPins[] =
{
//	char* Name;
//	TPortNumber PortNumber;
//	u8 PinNumber;
//	TCPUPin_Get CPUPin_Get;
//	TCPUPin_Set CPUPin_Set;
//	u32 Param;

    {"%null#", -1, -1, NULL, NULL, 0}, // нулевой элемент, для защиты при доступе от неприсвоенного номера pin

    // общая часть, 8 реле на плате
    {"V1", -1, 0, NULL, CPUPin_SerParSet, (u32) &ValvesPortDef},
    {"V2", -1, 1, NULL, CPUPin_SerParSet, (u32) &ValvesPortDef},
    {"V3", -1, 2, NULL, CPUPin_SerParSet, (u32) &ValvesPortDef},
    {"V4", -1, 3, NULL, CPUPin_SerParSet, (u32) &ValvesPortDef},
    {"V5", -1, 4, NULL, CPUPin_SerParSet, (u32) &ValvesPortDef},
    {"V6", -1, 5, NULL, CPUPin_SerParSet, (u32) &ValvesPortDef},
    {"V7", -1, 6, NULL, CPUPin_SerParSet, (u32) &ValvesPortDef},
    {"V8", -1, 7, NULL, CPUPin_SerParSet, (u32) &ValvesPortDef},

    // input for FRONT PANEL switches or other inputs (pull-upped)
    {"SW0", FRONT_PANEL_SW0_Port, FRONT_PANEL_SW0_Pin, CPUPin_DefaultGetInv, NULL, 0},
    {"SW1", FRONT_PANEL_SW1_Port, FRONT_PANEL_SW1_Pin, CPUPin_DefaultGetInv, NULL, 0},
    {"SW2", FRONT_PANEL_SW2_Port, FRONT_PANEL_SW2_Pin, CPUPin_DefaultGetInv, NULL, 0},
    {"SW3", FRONT_PANEL_SW3_Port, FRONT_PANEL_SW3_Pin, CPUPin_DefaultGetInv, NULL, 0},
    {"SW4", FRONT_PANEL_SW4_Port, FRONT_PANEL_SW4_Pin, CPUPin_DefaultGetInv, NULL, 0},

#ifdef WATER_STATION
    // HBridge
    {	"M1P", M1P_Port, M1P_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"M1M", M1M_Port, M1M_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"M2P", M2P_Port, M2P_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"M2M", M2M_Port, M2M_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"M3P", M3P_Port, M3P_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"M3M", M3M_Port, M3M_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"M4P", M4P_Port, M4P_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"M4M", M4M_Port, M4M_Pin, NULL, CPUPin_DefaultSet, 0},

    // управление клапанами на внешней плате для водной установки
    {	"VS1", -1, 0, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS2", -1, 1, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS3", -1, 2, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS4", -1, 3, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS5", -1, 4, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS6", -1, 5, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS7", -1, 6, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS8", -1, 7, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS9", -1, 8, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS10", -1, 9, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS11", -1, 10, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS12", -1, 11, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS13", -1, 12, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS14", -1, 13, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS15", -1, 14, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},
    {	"VS16", -1, 15, NULL, CPUPin_SerParSet, (u32) &ExternValvesPortDef},

    {	"HR1", HR1_Port, HR1_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"HR2", HR2_Port, HR2_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"HR3", HR3_Port, HR3_Pin, NULL, CPUPin_DefaultSet, 0},
    {	"HR4", HR4_Port, HR4_Pin, NULL, CPUPin_DefaultSet, 0},

#endif

#ifdef GAZ_STATION
    // четыре датчика
    {	"TEMP1", -1, 0, CPUPin_GetTemperature, NULL, 0},
    {	"PRESS1", -1, 0, CPUPin_GetPressure, NULL, 0},
    {	"OFFTEMP1", -1, 0, NULL, CPUPin_SetOffsetTemperature, 0},
    {	"OFFPRESS1", -1, 0, NULL, CPUPin_SetOffsetPressure, 0},

    {	"TEMP2", -1, 1, CPUPin_GetTemperature, NULL, 0},
    {	"PRESS2", -1, 1, CPUPin_GetPressure, NULL, 0},
    {	"OFFTEMP2", -1, 1, NULL, CPUPin_SetOffsetTemperature, 0},
    {	"OFFPRESS2", -1, 1, NULL, CPUPin_SetOffsetPressure, 0},

    {	"TEMP3", -1, 2, CPUPin_GetTemperature, NULL, 0},
    {	"PRESS3", -1, 2, CPUPin_GetPressure, NULL, 0},
    {	"OFFTEMP3", -1, 2, NULL, CPUPin_SetOffsetTemperature, 0},
    {	"OFFPRESS3", -1, 2, NULL, CPUPin_SetOffsetPressure, 0},

    {	"TEMP4", -1, 3, CPUPin_GetTemperature, NULL, 0},
    {	"PRESS4", -1, 3, CPUPin_GetPressure, NULL, 0},
    {	"OFFTEMP4", -1, 3, NULL, CPUPin_SetOffsetTemperature, 0},
    {	"OFFPRESS4", -1, 3, NULL, CPUPin_SetOffsetPressure, 0},

    // счетчики
    {	"METER1", -1, 0, CPUPin_Meter, NULL, 0},
    {	"METER2", -1, 1, CPUPin_Meter, NULL, 0},
    {	"METER3", -1, 2, CPUPin_Meter, NULL, 0},
    {	"METER4", -1, 3, CPUPin_Meter, NULL, 0},

    // передняя панель
    {	"LSTART", -1, 1, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef}, // start
    {	"LDONE", -1, 0, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef}, // done

    {	"L1G", -1, 9, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef}, // green led,	OK
    {	"L1R1", -1, 8, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef}, // red led,		HERMETIC
    {	"L1R2", -1, 11, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef}, // red led,		TESTING
    {	"L1R3", -1, 10, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef}, // red led,		MEASURING
    {	"L1SW", -1, 3, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef}, // switch led

    {	"L2G", -1, 21, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L2R1", -1, 20, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L2R2", -1, 23, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L2R3", -1, 2, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L2SW", -1, 6, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},

    {	"L3G", -1, 17, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L3R1", -1, 16, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L3R2", -1, 19, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L3R3", -1, 18, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L3SW", -1, 5, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},

    {	"L4G", -1, 13, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L4R1", -1, 12, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L4R2", -1, 15, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L4R3", -1, 14, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    {	"L4SW", -1, 4, NULL, CPUPin_SerParSet, (u32) &FrontPanelPortDef},
    // 6, 7 - свободные

#endif

#ifdef TERMO_STATION
    {"DS18B20_1", -1, 1, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_2", -1, 2, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_3", -1, 3, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_4", -1, 4, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_5", -1, 5, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_6", -1, 6, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_7", -1, 7, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_8", -1, 8, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_9", -1, 9, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_10", -1, 10, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_11", -1, 11, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_12", -1, 12, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_13", -1, 13, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_14", -1, 14, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_15", -1, 15, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_16", -1, 16, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_17", -1, 17, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_18", -1, 18, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_19", -1, 19, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_20", -1, 20, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_21", -1, 21, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_22", -1, 22, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_23", -1, 23, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_24", -1, 24, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_25", -1, 25, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_26", -1, 26, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_27", -1, 27, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_28", -1, 28, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_29", -1, 29, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_30", -1, 30, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_31", -1, 31, CPUPin_GetDS18B20Temperature, NULL, 0},
    {"DS18B20_32", -1, 32, CPUPin_GetDS18B20Temperature, NULL, 0},

//    {"IO00", IO00_Port, IO00_Pin, NULL, CPUPin_DefaultSet, 0},	// used for nRF
//    {"IO01", IO01_Port, IO01_Pin, NULL, CPUPin_DefaultSet, 0},	// used for nRF
//    {"IO02", IO02_Port, IO02_Pin, NULL, CPUPin_DefaultSet, 0},	// used for nRF
//    {"IO03", IO03_Port, IO03_Pin, NULL, CPUPin_DefaultSet, 0},	// used for nRF
//    {"IO04", IO04_Port, IO04_Pin, NULL, CPUPin_DefaultSet, 0},	// used for nRF
//    {"IO05", IO05_Port, IO05_Pin, NULL, CPUPin_DefaultSet, 0},	// used for nRF

    {"IO14", IO14_Port, IO14_Pin, NULL, CPUPin_DefaultSet, 0},
    {"IO15", IO15_Port, IO15_Pin, NULL, CPUPin_DefaultSet, 0},
    {"IO16", IO16_Port, IO16_Pin, NULL, CPUPin_DefaultSet, 0},
    {"IO17", IO17_Port, IO17_Pin, NULL, CPUPin_DefaultSet, 0},
    {"IO18", IO18_Port, IO18_Pin, NULL, CPUPin_DefaultSet, 0},
#endif

    {"IO20", IO20_Port, IO20_Pin, NULL, CPUPin_DefaultSet, 0},

//  { "ADC0", -1, 0, NULL, NULL, 0 },
//  { "ADC1", -1, 1, NULL, NULL, 0 },
//  { "ADC2", -1, 2, NULL, NULL, 0 },
//  { "ADC3", -1, 3, NULL, NULL, 0 },

    {NULL, 0, 0, NULL, NULL, 0} // завершитель списка
};

void InitCPU(void)
{
	printf("InitCPU, before\n\r");
	u32 i = 0;
	while (CPUPins[i].Name != NULL)
	{
		if (CPUPins[i].PortNumber != -1)
		{
			if (CPUPins[i].CPUPin_Get != NULL)
			{
				GPIO_SetDir(CPUPins[i].PortNumber, _BV(CPUPins[i].PinNumber), INPUT);
			}

			if (CPUPins[i].CPUPin_Set != NULL)
			{
				GPIO_Init(CPUPins[i].PortNumber, CPUPins[i].PinNumber, OUTPUT, false);
			}
		}
		i ++;
	}

	// временная заглушка, надо внести в настройки пина PULLUP для входных ножек
	GPIO_Init_Input_PullUp(FRONT_PANEL_SW0_Port, FRONT_PANEL_SW0_Pin);
	GPIO_Init_Input_PullUp(FRONT_PANEL_SW1_Port, FRONT_PANEL_SW1_Pin);
	GPIO_Init_Input_PullUp(FRONT_PANEL_SW2_Port, FRONT_PANEL_SW2_Pin);
	GPIO_Init_Input_PullUp(FRONT_PANEL_SW3_Port, FRONT_PANEL_SW3_Pin);
	GPIO_Init_Input_PullUp(FRONT_PANEL_SW4_Port, FRONT_PANEL_SW4_Pin);

	printf("InitCPU, after\n\r");
}

int FindCPUPinNumber(char* PinName)
{
	if (PinName != NULL)
	{
		int i = 0;
		while (CPUPins[i].Name != NULL)
		{
			if (strcmp(CPUPins[i].Name, PinName) == 0)
			{
				return (i);
			}
			i ++;
		}
	}
	return -1;
}

PCPUPin FindCPUPin(char* PinName)
{
	int pinnum = FindCPUPinNumber(PinName);

	if (pinnum != -1)
	{
		return ( &CPUPins[pinnum]);
	}
	return NULL;
}

//void SetCPUPinValue(char* PinName, u32 val)
//{
//	PCPUPin pin = FindCPUPin(PinName);
//	if (pin != NULL)
//	{
//		if (pin->CPUPin_Set != NULL)
//		{
//			pin->CPUPin_Set(pin, val);
//		}
//	}
//}

void SetCPUPinValue(int PinNumber, s32 val)
{
	PCPUPin pin = &CPUPins[PinNumber];
	if (pin != NULL)
	{
		if (pin->CPUPin_Set != NULL)
		{
			pin->CPUPin_Set(pin, val);
		}
	}
}

//u32 GetCPUPinValue(char* PinName)
//{
//	u32 res;
//	PCPUPin pin = FindCPUPin(PinName);
//	if (pin != NULL)
//	{
//		if (pin->CPUPin_Get != NULL)
//		{
//			res = pin->CPUPin_Get(pin);
//			return (res);
//		}
//	}
//	return 0;
//}

//s32 GetCPUPinValue(int PinNumber)
//{
//	s32 res;
//	PCPUPin pin = &CPUPins[PinNumber];
//	if (pin != NULL)
//	{
//		if (pin->CPUPin_Get != NULL)
//		{
//			res = pin->CPUPin_Get(pin);
//			return (res);
//		}
//	}
//	return 0;
//}

s32 CPUPin_DefaultGet(PCPUPin pin)
{
	if (pin != NULL)
	{
		if (GPIO_Read(pin->PortNumber, _BV(pin->PinNumber)))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

void CPUPin_DefaultSet(PCPUPin pin, s32 val)
{
	if (pin != NULL)
	{
		if (val)
		{
			GPIO_SetValue(pin->PortNumber, _BV(pin->PinNumber));
		}
		else
		{
			GPIO_ClearValue(pin->PortNumber, _BV(pin->PinNumber));
		}
	}
}

// инверсное чтение
//s32 CPUPin_DefaultGetInv(PCPUPin pin)
//{
//	return (1 - CPUPin_DefaultGet(pin));
//}

void CPUPin_DefaultGetInv(PCPUPin pin, char* buf, u8 bufsize)
{
	if (pin != NULL)
	{
		if (GPIO_Read(pin->PortNumber, _BV(pin->PinNumber)))
		{
//			return "0";
			snprintf(buf, bufsize, "0");
		}
		else
		{
//			return "1";
			snprintf(buf, bufsize, "1");
		}
	}
//	return "1";
	snprintf(buf, bufsize, "1");
}

void CPUPin_DefaultSetInv(PCPUPin pin, s32 val)
{
	CPUPin_DefaultSet(pin, (val == 0) ? 1 : 0);
}

void CPUPin_SerParSet(PCPUPin pin, s32 val)
{
	if (pin != NULL)
	{
		if (val)
		{
			SP_SetPin((PSerParPortDef) pin->Param, pin->PinNumber);
		}
		else
		{
			SP_ClearPin((PSerParPortDef) pin->Param, pin->PinNumber);
		}
	}
}

#ifdef GAZ_STATION
void CPUPin_GetTemperature(PCPUPin pin, char* buf, u8 bufsize)
{
	if (pin->PinNumber < SensCount)
	{
//		return Sens[pin->PinNumber].AvgTemp.Avg; // * 10, см. CalcPressTemp

		snprintf(buf, bufsize, "%.2f", GetTemperature(pin->PinNumber - 1));

		snprintf(buf, bufsize, "%f", Sens[pin->PinNumber].AvgTemp.Avg);
	}
	else
	{
		snprintf(buf, bufsize, "ERR");
	}
}

void CPUPin_GetPressure(PCPUPin pin, char* buf, u8 bufsize)
{
//	if (pin->PinNumber < SensCount)
//	{
//		return Sens[pin->PinNumber].AvgPress.Avg; // * 10, см. CalcPressTemp
//	}
//	return (s32) -1;

	if (pin->PinNumber < SensCount)
	{
		snprintf(buf, bufsize, "%f", Sens[pin->PinNumber].AvgPress.Avg);
	}
	else
	{
		snprintf(buf, bufsize, "ERR");
	}
}

void CPUPin_SetOffsetTemperature(PCPUPin pin, s32 val)
{
	if (pin->PinNumber < SensCount)
	{
		Sens[pin->PinNumber].H_Temp_Offset = val;
	}
}
void CPUPin_SetOffsetPressure(PCPUPin pin, s32 val)
{
	if (pin->PinNumber < SensCount)
	{
		Sens[pin->PinNumber].H_Press_Offset = val;
	}
}


// возвращаем просто номер счетчика
void CPUPin_Meter(PCPUPin pin, char* buf, u8 bufsize)
{
//	return (pin->PinNumber);
	snprintf(buf, bufsize, "%d", pin->PinNumber);
}
#endif

//s32 CPUPin_GetDS18B20Temperature(PCPUPin pin)
//{
//	return (s32)(GetTemperature(pin->PinNumber - 1) * 100);  // from 1..SensCount to 0..SensCount-1 range
//}

#ifdef TERMO_STATION
void CPUPin_GetDS18B20Temperature(PCPUPin pin, char* buf, u8 bufsize)
{
	snprintf(buf, bufsize, "%.2f", GetTemperature(pin->PinNumber - 1));
}
#endif

