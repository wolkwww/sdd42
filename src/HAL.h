//HAL.h

/*
 * HAL.h
 #include "HAL.h"


(20)
 *  Created on: 24.05.2010
 *      Author: hru
 */

#ifndef HAL_H_
#define HAL_H_

#include "LPC17xx.h"
#include "core_cm3.h"
#include "types.h"
#include "lpc_types.h"

//#define OUTPUT_72HZ
//#define BEEPER

///////////////////////////////////////////////////////////////////////////////////////////////

//DEBUG UART
//#define DEBUG_RX_Port					(0)
//#define DEBUG_RX_Pin					(1)
//#define DEBUG_RX_FUNC					(2)
//#define DEBUG_TX_Port					(0)
//#define DEBUG_TX_Pin					(0)
//#define DEBUG_TX_FUNC					(2)

// IO's
//#define IO06_Port						(0)
//#define IO06_Pin					_BV	(18)

//#define IO08_Port						(0)
//#define IO08_Pin					_BV (20)
//
//#define IO09_Port						(0)
//#define IO09_Pin					_BV (21)
//
//#define IO10_Port						(0)
//#define IO10_Pin					_BV (22)

//#define IO11_Port						(0)
//#define IO11_Pin					_BV (27)
//
//#define IO12_Port						(0)
//#define IO12_Pin					_BV (28)
//
//#define IO13_Port						(0)
//#define IO13_Pin					_BV (29)
//
//#define IO14_Port						(0)
//#define IO14_Pin					_BV (30)
//
//#define IO15_Port						(1)
//#define IO15_Pin					_BV (0)
//
//#define IO16_Port						(1)
//#define IO16_Pin					_BV (1)
//
//#define IO17_Port						(1)
//#define IO17_Pin					_BV (4)
//
//#define IO18_Port						(1)
//#define IO18_Pin					_BV (8)
//
//#define IO19_Port						(1)
//#define IO19_Pin					_BV (9)
//
//#define IO20_Port						(1)
//#define IO20_Pin					_BV (10)
//
//#define IO21_Port						(1)
//#define IO21_Pin					_BV (14)
//
//#define IO22_Port						(1)
//#define IO22_Pin					_BV (15)
//
//#define IO23_Port						(1)
//#define IO23_Pin					_BV (16)
//
//#define IO24_Port						(1)
//#define IO24_Pin					_BV (17)
//
//#define IO25_Port						(1)
//#define IO25_Pin					_BV (20)
//
//#define IO26_Port						(1)
//#define IO26_Pin					_BV (21)
//
//#define IO27_Port						(1)
//#define IO27_Pin					_BV (22)
//
//#define IO28_Port						(1)
//#define IO28_Pin					_BV (23)
//
//#define IO29_Port						(1)
//#define IO29_Pin					_BV (24)
//
//#define IO30_Port						(1)
//#define IO30_Pin					_BV (25)
//
//#define IO31_Port						(1)
//#define IO31_Pin					_BV (28)
//
//#define IO32_Port						(1)
//#define IO32_Pin					_BV (29)
//
//#define IO33_Port						(2)
//#define IO33_Pin					_BV (0)
//
//#define IO34_Port						(2)
//#define IO34_Pin					_BV (1)
//
//#define IO35_Port						(3)
//#define IO35_Pin					_BV (26)
//
//#define IO36_Port						(4)
//#define IO36_Pin					_BV (28)
//
//#define IO37_Port						(4)
//#define IO37_Pin					_BV (29)

//ADC inputs
//#define AIN0_Port						(0)
//#define AIN0_Pin						(23)
//
//#define AIN1_Port						(0)
//#define AIN1_Pin						(24)
//
//#define AIN2_Port						(0)
//#define AIN2_Pin						(25)
//
//#define AIN3_Port						(0)
//#define AIN3_Pin						(26)
//
//#define AIN4_Port						(1)
//#define AIN4_Pin						(30)
//
//#define AIN5_Port						(1)
//#define AIN5_Pin						(31)
//
//#define AIN6_Port						(0)
//#define AIN6_Pin						(3)
//
//#define AIN7_Port						(0)
//#define AIN7_Pin						(2)

//input for FRONT PANEL switches
#define FRONT_PANEL_SW0_Port		(2)
#define FRONT_PANEL_SW0_Pin			(1)

#define FRONT_PANEL_SW1_Port		(2)
#define FRONT_PANEL_SW1_Pin			(2)

#define FRONT_PANEL_SW2_Port		(2)
#define FRONT_PANEL_SW2_Pin			(3)

#define FRONT_PANEL_SW3_Port		(2)
#define FRONT_PANEL_SW3_Pin			(4)

#define FRONT_PANEL_SW4_Port		(2)
#define FRONT_PANEL_SW4_Pin			(5)

//?????????????????????
//#define OUT01_Port						(2)
//#define OUT01_Pin						(2)
//
//#define OUT02_Port						(2)
//#define OUT02_Pin						(3)
//
//#define OUT03_Port						(2)
//#define OUT03_Pin						(4)
//
//#define OUT04_Port						(2)
//#define OUT04_Pin						(5)
//
//#define OUT05_Port						(2)
//#define OUT05_Pin						(6)
//
//#define OUT06_Port						(2)
//#define OUT06_Pin						(7)
//
//#define OUT07_Port						(2)
//#define OUT07_Pin						(8)
//
//#define OUT08_Port						(2)
//#define OUT08_Pin						(9)

//         IOs
/////////////////////////////////////////////////////////////////
#define IO00_Port							(0)
#define IO00_Pin							(23)

#define IO01_Port							(0)
#define IO01_Pin							(24)

#define IO02_Port							(0)
#define IO02_Pin							(25)

#define IO03_Port							(0)
#define IO03_Pin							(26)

#define IO04_Port							(0)
#define IO04_Pin							(27)

#define IO05_Port							(0)
#define IO05_Pin							(28)

#define IO06_Port							(0)
#define IO06_Pin							(29)

#define IO07_Port							(0)
#define IO07_Pin							(30)

#define IO08_Port							(1)
#define IO08_Pin							(16)

#define IO09_Port							(1)
#define IO09_Pin							(17)

#define IO10_Port							(1)
#define IO10_Pin							(18)

#define IO11_Port							(1)
#define IO11_Pin							(20)

#define IO12_Port							(1)
#define IO12_Pin							(21)

#define IO13_Port							(1)
#define IO13_Pin							(22)

#define IO14_Port							(1)
#define IO14_Pin							(23)

#define IO15_Port							(1)
#define IO15_Pin							(24)

#define IO16_Port							(1)
#define IO16_Pin							(25)

#define IO17_Port							(1)
#define IO17_Pin							(26)

#define IO18_Port							(2)
#define IO18_Pin							(11)

#define IO19_Port							(2)
#define IO19_Pin							(12)

#define IO20_Port							(2)
#define IO20_Pin							(13)

#define IO21_Port							(3)
#define IO21_Pin							(26)

// ETHERNET SPI

//MMC_CS_
#define ETHER_CE_Port						(0)
#define ETHER_CE_Pin						(6)
#define ETHER_CE_FUNC						(0)

//MMC_CLK_
#define ETHER_SCK_Port						(0)
#define ETHER_SCK_Pin						(7)
#define ETHER_SCK_FUNC						(2)

//MMC_DATA_OUT_
#define ETHER_MISO_Port						(0)
#define ETHER_MISO_Pin						(8)
#define ETHER_MISO_FUNC						(2)

//MMC_DATA_IN_
#define ETHER_MOSI_Port						(0)
#define ETHER_MOSI_Pin						(9)
#define ETHER_MOSI_FUNC						(2)

// External Timer

#define EXT_TIMER_Port						(1)
#define EXT_TIMER_Pin					_BV (19)

//#define _Port						()
//#define _Pin					_BV ()

// LEDS
#define LED1_Port							(4)
#define LED1_Pin							(28)

#define LED2_Port							(4)
#define LED2_Pin							(29)

#define LED1_OFF()							GPIO_SetValue(LED1_Port, _BV(LED1_Pin))
#define LED1_ON()							GPIO_ClearValue(LED1_Port, _BV(LED1_Pin))

#define LED2_OFF()							GPIO_SetValue(LED2_Port, _BV(LED2_Pin))
#define LED2_ON()							GPIO_ClearValue(LED2_Port, _BV(LED2_Pin))

#ifdef OUTPUT_72HZ
#define OUTPUT_72HZ_PORT					IO09_Port
#define OUTPUT_72HZ_PIN						IO09_Pin
#endif

#ifdef BEEPER
#define BEEPER_PORT							IO08_Port
#define BEEPER_PIN							IO08_Pin
#endif

//#define OUTPORT_SERIAL_CLOCK_Port			IO07_Port
//#define OUTPORT_SERIAL_CLOCK_Pin			IO07_Pin
#define OUTPORT_SERIAL_CLOCK_Port			IO10_Port
#define OUTPORT_SERIAL_CLOCK_Pin			IO10_Pin

#define OUTPORT_SERIAL_DATA_Port			IO08_Port
#define OUTPORT_SERIAL_DATA_Pin				IO08_Pin

#define OUTPORT_LATCH_CLOCK_Port			IO09_Port
#define OUTPORT_LATCH_CLOCK_Pin				IO09_Pin

#define M2P_Port							IO11_Port
#define M2P_Pin								IO11_Pin

#define M4M_Port							IO12_Port
#define M4M_Pin								IO12_Pin

#define M1P_Port							IO13_Port
#define M1P_Pin								IO13_Pin

#define M3M_Port							IO14_Port
#define M3M_Pin								IO14_Pin

#define M1M_Port							IO15_Port
#define M1M_Pin								IO15_Pin

#define M4P_Port							IO16_Port
#define M4P_Pin								IO16_Pin

#define M2M_Port							IO17_Port
#define M2M_Pin								IO17_Pin

#define M3P_Port							IO18_Port
#define M3P_Pin								IO18_Pin

#define HR1_Port							(0)
#define HR1_Pin								(21)

#define HR2_Port							(0)
#define HR2_Pin								(17)

#define HR3_Port							(0)
#define HR3_Pin								(22)

#define HR4_Port							(0)
#define HR4_Pin								(18)


//#define TEST1_OUT_PORT						(1)
//#define TEST1_OUT_PIN						(20)
//
//#define TEST2_OUT_PORT						(1)
//#define TEST2_OUT_PIN						(21)
//
//#define TEST1_ON()							GPIO_SetValue(TEST1_OUT_PORT, _BV(TEST1_OUT_PIN))
//#define TEST1_OFF()							GPIO_ClearValue(TEST1_OUT_PORT, _BV(TEST1_OUT_PIN))
//
//#define TEST2_ON()							GPIO_SetValue(TEST2_OUT_PORT, _BV(TEST2_OUT_PIN))
//#define TEST2_OFF()							GPIO_ClearValue(TEST2_OUT_PORT, _BV(TEST2_OUT_PIN))

/////////////////////////////////////////////////////////

//#define GPIO_Set(port, pin)		GPIO_SetValue	(port, pin)
//#define GPIO_Reset(port, pin)	GPIO_ClearValue (port, pin)
//#define GPIO_Invert(port, pin)	(GPIO_Read(port, pin) ? GPIO_ClearValue(port, pin) : GPIO_SetValue(port, pin))

#define __START_ADDRESS__	0

#define GPIO_Read(port, pin)	((GPIO_ReadValue(port) & pin) != 0)

void GPIO_Init_Input_PullUp(u32 port, u32 pin);
void GPIO_Init(u32 port, u32 pin, u32 dir, bool initial_state);
void GPIO_Init_OpenDrain(u32 port, u32 pin, u32 dir, bool initial_state);

// ����� ������ �������� ����������
//#define __START_ADDRESS__	0

//Watchodog time out in 2 seconds
#define WDT_TIMEOUT 	5000000

typedef enum
{
	INPUT = 0,
	OUTPUT = 1
} PortDirection;

// ������ ������������ ���������� �������, ���
#define SYS_TIMER_PERIOD_US		50	// = 20kHz
// ��������� ���������� ������� � 10���
//#define TIMER0_PRESCALLER		(100 / SYS_TIMER_PERIOD_US)

/////////////////////////////////////////////////////////
//UART pins
/////////////////////////////////////////////////////////

extern volatile bool BeepEnable;

u64 GetTicksMs(void);
u64 GetTicks(void);
u64 GetUptime(void);
u32 GetRTC(void);
void SetRTC(u32 new_rtc);

void delay(u32 tick);

void SysInit(void);

void PortInit(void);

void WDTInit(void);
void WDTEnable(void);
void WDTReset(void);

void CLKInit(void);

void EEPROMInit(void);

void TC0Init(void);

void DACInit(void);
void DACDeinit(void);
void DACOut(u32 val);

void VICEnable(void);
void IntsEnable(void);	// enable global interrupts
void IntsDisable(void);	// disable global interrupts

void DBGResetReason(void);

#endif /* HAL_H_ */
