/*
#include "LowTimers.h"
 *
 *  Created on: 07.11.2013
 *      Author: mrhru
 */

#ifndef LOWTIMERS_H_
#define LOWTIMERS_H_

typedef u64 LowTimer;

void InitTimer(LowTimer* ptm);
u64 GetTimerMs(LowTimer tm);


#endif /* LOWTIMERS_H_ */
