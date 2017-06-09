/*
 * DS18B20.h
 *
 *  Created on: 12.09.2014
 *      Author: mrhru
 */

#ifndef DS18B20_H_
#define DS18B20_H_

#include "defines.h"
#ifdef TERMO_STATION

// старое подключение, сейчас занято nRF
//#define DS18B20_DATA_PORT           	IO05_Port
//#define DS18B20_DATA_PIN				IO05_Pin

// новое P2[5], разъем JP_FRONT_PANEL, ножка 15  FRONT_PANEL_NSW4
#define DS18B20_DATA_PORT           	(2)
#define DS18B20_DATA_PIN				(5)


//#define THERM_INPUT_MODE()  (THERM_PORT->DDR &= ~THERM_PIN)
//#define THERM_OUTPUT_MODE() (THERM_PORT->DDR |=  THERM_PIN)
//#define THERM_LOW()         GPIOResetBits(THERM_PORT, THERM_PIN)
//#define THERM_HIGH()        GPIOSetBits(THERM_PORT, THERM_PIN)
//#define THERM_READ()        (THERM_PORT->IDR & (vu8)THERM_PIN)

#define THERM_INPUT_MODE()  		GPIO_SetDir		(DS18B20_DATA_PORT, _BV(DS18B20_DATA_PIN), INPUT)
#define THERM_OUTPUT_MODE() 		GPIO_SetDir		(DS18B20_DATA_PORT, _BV(DS18B20_DATA_PIN), OUTPUT)
#define THERM_LOW()         		GPIO_ClearValue	(DS18B20_DATA_PORT, _BV(DS18B20_DATA_PIN))
#define THERM_HIGH()        		GPIO_SetValue	(DS18B20_DATA_PORT, _BV(DS18B20_DATA_PIN))
#define THERM_READ()        		GPIO_Read		(DS18B20_DATA_PORT, _BV(DS18B20_DATA_PIN))

void delay_1us(void);
void delay_15us(void);
void delay_45us(void);
void delay_60us(void);
void delay_480us(void);

extern bool DS18B20_Enabled;

void DS18B20_Init(void);
void DS18B20_process(void);
float GetTemperature(int devNum);

#endif

#endif /* DS18B20_H_ */
