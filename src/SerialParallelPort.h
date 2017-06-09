//SerialParallelPort.h
/*
 * SerialParallelPort.h
 *
 *  Created on: 05.03.2013
 *      Author: mrhru
 */

#ifndef SERIALPARALLELPORT_H_
#define SERIALPARALLELPORT_H_

#include "types.h"
#include "common.h"
#include "HAL.h"

//input for FRONT PANEL switches
//#define FP_SW1_Port					AIN3_Port
//#define FP_SW1_bit					AIN3_bit
//
//#define FP_SW2_Port					AIN4_Port
//#define FP_SW2_bit					AIN4_bit
//
//#define FP_SW3_Port					AIN5_Port
//#define FP_SW3_bit					AIN5_bit
//
//#define FP_SW4_Port					AIN6_Port
//#define FP_SW4_bit					AIN6_bit
//
//#define FP_SW5_Port					AIN7_Port
//#define FP_SW5_bit					AIN7_bit

//#define FP_SW0_Port					(0)
//#define FP_SW0_bit					(2)
//
//#define FP_SW1_Port					(1)
//#define FP_SW1_bit					(30)
//
//#define FP_SW2_Port					(0)
//#define FP_SW2_bit					(26)
//
//#define FP_SW3_Port					(0)
//#define FP_SW3_bit					(3)
//
//#define FP_SW4_Port					(1)
//#define FP_SW4_bit					(31)


struct tagSerParPinDef
{
	u8 Port;
	u8 Bit;
};
typedef struct tagSerParPinDef TSerParPinDef;

struct tagSerParPortDef
{
	TSerParPinDef LATCH;
	TSerParPinDef DATA;
	TSerParPinDef CLOCK;

	u8 BYTE_COUNT;
	bool INVERSE;

	u8* OutputBytes;
};
typedef struct tagSerParPortDef TSerParPortDef;
typedef TSerParPortDef* PSerParPortDef;


void SP_Init(PSerParPortDef port);
void SP_SetPin(PSerParPortDef port, u32 pinnum);
void SP_ClearPin(PSerParPortDef port, u32 pinnum);
void SP_TestSet(PSerParPortDef port);

extern TSerParPortDef FrontPanelPortDef;	// для управления таблом на газовой установке
extern TSerParPortDef ValvesPortDef;		// для встроенных в плату контроллера реле
extern TSerParPortDef ExternValvesPortDef;	// для управления клапанами водяной установки

#endif /* SERIALPARALLELPORT_H_ */
