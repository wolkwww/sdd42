/*
 * DS18B20Tray.h
 *
 *  Created on: 26 мая 2017 г.
 *      Author: mrhru
 */

#ifndef DS18B20TRAY_H_
#define DS18B20TRAY_H_

#include "defines.h"
#ifdef TERMO_STATION

#include "CPU.h"

void TrayInit(void);
void TrayTest(void);

extern const struct TType TTray;

#endif

#endif /* DS18B20TRAY_H_ */

