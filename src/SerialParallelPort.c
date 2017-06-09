//SerialParallelPort.c
/*
 * SerialParallelPort.c
 *
 *  Created on: 05.03.2013
 *      Author: mrhru
 */

#include <stdio.h>

#include "Utils.h"
#include "SerialParallelPort.h"
#include "lpc17xx_pinsel.h"

////serial-paraller port for FRONT PANEL LEDs
//#define FRONT_PANEL_LATCH_Port	(1)
//#define FRONT_PANEL_LATCH_bit		(30)
//
//#define FRONT_PANEL_DATA_Port		(1)
//#define FRONT_PANEL_DATA_bit		(31)
//
//#define FRONT_PANEL_CLOCK_Port	(2)
//#define FRONT_PANEL_CLOCK_bit		(0)
//
//#define SP_PIN_COUNT				24
//#define SP_BYTE_COUNT				3	//(SP_PIN_COUNT >> 3)
//u8 OutputBytes[SP_BYTE_COUNT];

TSerParPortDef FrontPanelPortDef =
{
{1, 30}, //
    {1, 31}, //
    {2, 0}, //
    3,	//
    TRUE,	//
    };

TSerParPortDef ValvesPortDef =
{
{1, 29},	//
    {1, 27},	//
    {1, 28},	//
    1,	//
    FALSE,	//
    };

TSerParPortDef ExternValvesPortDef =
{
	{OUTPORT_LATCH_CLOCK_Port, OUTPORT_LATCH_CLOCK_Pin},	//
    {OUTPORT_SERIAL_DATA_Port, OUTPORT_SERIAL_DATA_Pin},	//
    {OUTPORT_SERIAL_CLOCK_Port, OUTPORT_SERIAL_CLOCK_Pin},	//
    2,		//
    FALSE,	//
    };

void SendOutputBytes(PSerParPortDef port);

void SP_Init(PSerParPortDef port)
{
	void InitOUTPUTGPIO(u32 port, u32 pin)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 0;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
		PinCfg.Portnum = port;
		PinCfg.Pinnum = pin;
		PINSEL_ConfigPin( &PinCfg);
		GPIO_SetDir(port, _BV(pin), OUTPUT);
		GPIO_ClearValue(port, _BV(pin));
	}

	InitOUTPUTGPIO(port->LATCH.Port, port->LATCH.Bit);
	InitOUTPUTGPIO(port->DATA.Port, port->DATA.Bit);
	InitOUTPUTGPIO(port->CLOCK.Port, port->CLOCK.Bit);

	port->OutputBytes = getmem(port->BYTE_COUNT, "SerPar Init");

	for (int i = 0; i < port->BYTE_COUNT; i ++)
	{
		port->OutputBytes[i] = port->INVERSE == TRUE ? 0xFF : 0x00;
	}
	SendOutputBytes(port);
}

void SendOutputBytes(PSerParPortDef port)
{
	void SP_Delay(void)
	{
		volatile int i;
		for (i = 0; i < 40; i ++)
			;
	}

	void SP_CLOCK(void)
	{
		SP_Delay();
		GPIO_SetValue(port->CLOCK.Port, _BV(port->CLOCK.Bit));
		SP_Delay();
		GPIO_ClearValue(port->CLOCK.Port, _BV(port->CLOCK.Bit));
	}

	void SP_DATA(bool bit)
	{
		if (bit)
		{
			GPIO_SetValue(port->DATA.Port, _BV(port->DATA.Bit));
		}
		else
		{
			GPIO_ClearValue(port->DATA.Port, _BV(port->DATA.Bit));
		}
		SP_Delay();
	}

	void SP_LATCH(void)
	{
		SP_Delay();
		GPIO_SetValue(port->LATCH.Port, _BV(port->LATCH.Bit));
		SP_Delay();
		GPIO_ClearValue(port->LATCH.Port, _BV(port->LATCH.Bit));
	}

	void SendOutputByte(u8 b)
	{
		for (int i = 0x80; i != 0; i >>= 1)
		{
			SP_DATA(b & i);
			SP_CLOCK();
		}
	}

	for (int i = port->BYTE_COUNT - 1; i >= 0; i --)
	{
		SendOutputByte(port->OutputBytes[i]);
	}
//	for (int i = 0; i < port->BYTE_COUNT; i++)
//	{
//		SendOutputByte(port->OutputBytes[i]);
//	}

	SP_LATCH();
}

void SP_ClearPin(PSerParPortDef port, u32 pinnum)
{
	u32 pos = pinnum / 8;
	u32 bit = pinnum & 0x07;

	if ( !port->INVERSE)
	{
		port->OutputBytes[pos] &= ~_BV(bit);
	}
	else
	{
		port->OutputBytes[pos] |= _BV(bit);
	}

	SendOutputBytes(port);
}

void SP_SetPin(PSerParPortDef port, u32 pinnum)
{
	u32 pos = pinnum / 8;
	u32 bit = pinnum & 0x07;

	if ( !port->INVERSE)
	{
		port->OutputBytes[pos] |= _BV(bit);
	}
	else
	{
		port->OutputBytes[pos] &= ~_BV(bit);
	}

	SendOutputBytes(port);
}

void SP_TestSet(PSerParPortDef port)
{
	printf("SP_TestSet, bytes = %u\n", port->BYTE_COUNT);
	u32 prev = 0;
	while (1)
	{
		for (u32 next = 0; next < port->BYTE_COUNT * 8; next ++)
		{
			SP_ClearPin(port, prev);
			SP_SetPin(port, next);
			prev = next;

			if ((next % 2) != 0)
			{
				LED1_ON();
				LED2_OFF();
			}
			else
			{
				LED2_ON();
				LED1_OFF();
			}
			delay(100);

		}
	}
}
