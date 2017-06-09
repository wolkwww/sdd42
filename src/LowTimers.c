/*
 * LowTimers.c
 *
 *  Created on: 07.11.2013
 *      Author: mrhru
 */

#include "types.h"
#include "common.h"
#include "HAL.h"

#include "LowTimers.h"

void InitTimer(LowTimer* ptm)
{
	*ptm = GetTicks() / 10;
}

u64 GetTimerMs(LowTimer tm)
{
	return (GetTicks() / 10 - tm);
}
