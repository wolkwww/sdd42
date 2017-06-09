// CPU.h

#ifndef _CPU_H
#define _CPU_H

#include "types.h"
#include "common.h"

struct TCPUPin;
typedef const struct TCPUPin* PCPUPin;

//typedef s32 (*TCPUPin_Get)(PCPUPin pin);
//typedef void (*TCPUPin_Set)(PCPUPin pin, s32 val);
typedef void (*TCPUPin_Get)(PCPUPin pin, char* buf, u8 bufsize);
typedef void (*TCPUPin_Set)(PCPUPin pin, s32 val);

#ifdef 		STM8SL
typedef GPIO_TypeDef* TPortNumber;
#else
#ifdef STM32W
typedef u8 TPortNumber;
#endif
#endif

typedef u8 TPortNumber;

struct TCPUPin
{
	char* Name;
	TPortNumber PortNumber;
	u8 PinNumber;
	TCPUPin_Get CPUPin_Get;
	TCPUPin_Set CPUPin_Set;
	u32 Param;
};

//void SetCPUPinValue(char* PinName, u32 val);
void SetCPUPinValue(int PinNumber, s32 val);
//u32 GetCPUPinValue(char* PinName);
s32 GetCPUPinValue(int PinNumber);
int FindCPUPinNumber(char* PinName);
PCPUPin FindCPUPin(char* PinName);

extern const struct TCPUPin CPUPins[];

void InitCPU(void);

#endif
