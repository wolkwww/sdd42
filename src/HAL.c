/*
 * HAL.c
 #include "HAL.h"
 *
 *  Created on: 24.05.2010
 *      Author: hru
 */

#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lpc17xx_libcfg.h"
#include "lpc17xx.h"
#include "lpc_types.h"
#include "core_cm3.h"
#include "lpc17xx_wdt.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_wdt.h"

#include "common.h"
#include "uart_driver.h"
//#include "UART_DEBUG.h"

#include "HAL.h"

volatile bool BeepEnable = FALSE;

void NVICInit(void);

void SysTimerInit(u32 Period_us);

void GPIO_Init_Input_PullUp(u32 port, u32 pin)
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
	PinCfg.Portnum = port;
	PinCfg.Pinnum = pin;
	PINSEL_ConfigPin( &PinCfg);

	GPIO_SetDir(port, _BV(pin), INPUT);
}

void GPIO_Init(u32 port, u32 pin, u32 dir, bool initial_state)
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Portnum = port;
	PinCfg.Pinnum = pin;
	PINSEL_ConfigPin( &PinCfg);

	GPIO_SetDir(port, _BV(pin), dir);

	if (initial_state)
	{
		GPIO_SetValue(port, _BV(pin));
	}
	else
	{
		GPIO_ClearValue(port, _BV(pin));
	}
}

void GPIO_Init_OpenDrain(u32 port, u32 pin, u32 dir, bool initial_state)
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL; //PINSEL_PINMODE_OPENDRAIN;
	PinCfg.Pinmode = PINSEL_PINMODE_PULLUP; // O!!! Оно!!!
	PinCfg.Portnum = port;
	PinCfg.Pinnum = pin;
	PINSEL_ConfigPin( &PinCfg);

	GPIO_SetDir(port, _BV(pin), dir);

	if (initial_state)
	{
		GPIO_SetValue(port, _BV(pin));
	}
	else
	{
		GPIO_ClearValue(port, _BV(pin));
	}
}

// SysTick Counter
static volatile u64 SysTickCnt = 0;			// 1 / 10000 s = 1/10 ms

static volatile u32 one10000sec = 0;		// счетчик 0.1 миллисекунд
static volatile u32 uptime = 0;
static volatile time_t rtc_sec = 1041422400U;	// Real Time Clock (start at 2003/01/01 12:00:00)

#ifdef OUTPUT_72HZ
static volatile u32 timer72 = 10;
static volatile bool timer72_state = FALSE;
#define COUNTER_72HZ		(10000 / 2 / 72)	// for 72 Hz output
#endif

#define COUNTER_BEEP		(10000 / 2 / 2000)	// for beeper output (2 kHz)
static volatile u32 beeper_timer = 1;
static volatile bool beeper_state = FALSE;

/* SysTick Interrupt Handler (0.1ms)    */
void SysTick_Handler(void)
{
	SysTickCnt ++;

#ifdef OUTPUT_72HZ
	if (timer72 == 0)
	{
		timer72 = COUNTER_72HZ;
		if (timer72_state == FALSE)
		{
			GPIO_SetValue(OUTPUT_72HZ_PORT, _BV(OUTPUT_72HZ_PIN));
			timer72_state = TRUE;
		}
		else
		{
			GPIO_ClearValue(OUTPUT_72HZ_PORT, _BV(OUTPUT_72HZ_PIN));
			timer72_state = FALSE;
		}
	}
	timer72--;
#endif

#ifdef BEEPER
	if (BeepEnable != FALSE)
	{
		if (beeper_timer == 0)
		{
			beeper_timer = COUNTER_BEEP;
			if (beeper_state == FALSE)
			{
				GPIO_SetValue(BEEPER_PORT, _BV(BEEPER_PIN));
				beeper_state = TRUE;
			}
			else
			{
				GPIO_ClearValue(BEEPER_PORT, _BV(BEEPER_PIN));
				beeper_state = FALSE;
			}
		}
		beeper_timer--;
	}
#endif

	if ( ++one10000sec >= 9999)
	{
		one10000sec = 0;
		rtc_sec ++;
		uptime ++;
	}
}

/*********************************************************************//**
 * @brief		Reading Ticks in milliseconds
 * @return 		ticks (ms)
 **********************************************************************/
u64 GetTicksMs(void)
{
	return GetTicks() / 10;
}

/*********************************************************************//**
 * @brief		Reading SysTickCnt, 1/10 millisecond
 * @return 		tick (1/10 ms)
 **********************************************************************/
u64 GetTicks(void)
{
	// SysTickCnt у нас длинный и изменяется в прерывании
	u64 res = SysTickCnt;
	// если два последовательных чтения дают разный результат
	while (res != SysTickCnt)
	{
		// то читаем еще раз
		res = SysTickCnt;
	}
	return res;
}

u64 GetUptime(void)
{
	return uptime;
}

u32 GetRTC(void)
{
	return rtc_sec;
}

void SetRTC(u32 new_rtc)
{
	rtc_sec = new_rtc;
}

/*********************************************************************//**
 * @brief		Delay millisecond
 * @param[in]	tick (ms)
 * @return 		None
 **********************************************************************/
void delay(u32 tick_ms)
{
	u64 systickcnt;
	tick_ms *= 10;

	systickcnt = GetTicks();
	while ((GetTicks() - systickcnt) < tick_ms)
	{
		WDTReset();
	}
}

void PortInit(void)
{
	void InitINPUTGPIO(u32 port, u32 pin)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 0;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
		PinCfg.Portnum = port;
		PinCfg.Pinnum = pin;
		PINSEL_ConfigPin( &PinCfg);
		GPIO_SetDir(port, _BV(pin), INPUT);
	}

	// NB!!!
	// ножка P2[10] должна воспринимать 1 при старте процессора
	// на первых платах она закорочена на 3V3, поэтому настраиваем
	// ее всегда на ввод, что бы ничего не случилось.
	GPIO_SetDir(2, _BV(10), INPUT);

	//Отключаем прерывания по EXT1, EXT2 и EXT3
	LPC_SC->EXTINT = 0x0;
	LPC_SC->EXTMODE = 0x0;
	LPC_SC->EXTPOLAR = 0x0;

//	GPIO_Init(OUT01_Port, OUT01_bit, OUTPUT, false);
//	GPIO_Init(OUT02_Port, OUT02_bit, OUTPUT, false);
//	GPIO_Init(OUT03_Port, OUT03_bit, OUTPUT, false);
//	GPIO_Init(OUT04_Port, OUT04_bit, OUTPUT, false);
//	GPIO_Init(OUT05_Port, OUT05_bit, OUTPUT, false);
//	GPIO_Init(OUT06_Port, OUT06_bit, OUTPUT, false);

	InitINPUTGPIO(FRONT_PANEL_SW0_Port, FRONT_PANEL_SW0_Pin);
	InitINPUTGPIO(FRONT_PANEL_SW1_Port, FRONT_PANEL_SW1_Pin);
	InitINPUTGPIO(FRONT_PANEL_SW2_Port, FRONT_PANEL_SW2_Pin);
	InitINPUTGPIO(FRONT_PANEL_SW3_Port, FRONT_PANEL_SW3_Pin);
	InitINPUTGPIO(FRONT_PANEL_SW4_Port, FRONT_PANEL_SW4_Pin);

	// LEDS
	GPIO_Init(LED1_Port, LED1_Pin, OUTPUT, true);
	GPIO_Init(LED2_Port, LED2_Pin, OUTPUT, true);

	// DigOUT Port default as Output
	//TODO ...
//	GPIO_SetDir(DOUT0_Port, _BV(DOUT0_bit), OUTPUT); //пока не работает, т.к. открытый коллектор
//	GPIO_SetDir(DOUT1_Port, _BV(DOUT1_bit), OUTPUT); //пока не работает, т.к. тоже открытый коллектор
//

#ifdef OUTPUT_72HZ
	GPIO_Init(OUTPUT_72HZ_PORT, OUTPUT_72HZ_PIN, OUTPUT, FALSE);
#endif

#ifdef BEEPER
	GPIO_Init(BEEPER_PORT, BEEPER_PIN, OUTPUT, FALSE);
#endif

	GPIO_Init(OUTPORT_SERIAL_CLOCK_Port, OUTPORT_SERIAL_CLOCK_Pin, OUTPUT, FALSE);
	GPIO_Init(OUTPORT_SERIAL_DATA_Port, OUTPORT_SERIAL_DATA_Pin, OUTPUT, FALSE);
	GPIO_Init(OUTPORT_LATCH_CLOCK_Port, OUTPORT_LATCH_CLOCK_Pin, OUTPUT, FALSE);

	// TODO: test
//	while(1)
//	{
//		GPIO_SetValue(OUTPORT_SERIAL_CLOCK_Port, _BV(OUTPORT_SERIAL_CLOCK_Pin));
//		GPIO_SetValue(OUTPORT_SERIAL_DATA_Port, _BV(OUTPORT_SERIAL_DATA_Pin));
//		GPIO_SetValue(OUTPORT_LATCH_CLOCK_Port, _BV(OUTPORT_LATCH_CLOCK_Pin));
//		LED1_ON();
//		LED2_OFF();
//		delay(100);
//
//		GPIO_ClearValue(OUTPORT_SERIAL_CLOCK_Port, _BV(OUTPORT_SERIAL_CLOCK_Pin));
//		GPIO_ClearValue(OUTPORT_SERIAL_DATA_Port, _BV(OUTPORT_SERIAL_DATA_Pin));
//		GPIO_ClearValue(OUTPORT_LATCH_CLOCK_Port, _BV(OUTPORT_LATCH_CLOCK_Pin));
//		LED2_ON();
//		LED1_OFF();
//		delay(100);
//	}

	GPIO_Init(M2P_Port, M2P_Pin, OUTPUT, FALSE);
	GPIO_Init(M4M_Port, M4M_Pin, OUTPUT, FALSE);
	GPIO_Init(M1P_Port, M1P_Pin, OUTPUT, FALSE);
	GPIO_Init(M3M_Port, M3M_Pin, OUTPUT, FALSE);
	GPIO_Init(M1M_Port, M1M_Pin, OUTPUT, FALSE);
	GPIO_Init(M4P_Port, M4P_Pin, OUTPUT, FALSE);
	GPIO_Init(M2M_Port, M2M_Pin, OUTPUT, FALSE);
	GPIO_Init(M3P_Port, M3P_Pin, OUTPUT, FALSE);

	GPIO_Init(HR1_Port, HR1_Pin, OUTPUT, FALSE);
	GPIO_Init(HR2_Port, HR2_Pin, OUTPUT, FALSE);
	GPIO_Init(HR3_Port, HR3_Pin, OUTPUT, FALSE);
	GPIO_Init(HR4_Port, HR4_Pin, OUTPUT, FALSE);
}

void DACInit(void)
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;	// DAC
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin( &PinCfg);
	DAC_Init(LPC_DAC);
}

void DACDeinit(void)
{
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 1; //ADC 0.3
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin( &PinCfg);
}

void DACOut(u32 val)
{
	DAC_UpdateValue(LPC_DAC, val);
}

void SysInit(void)
{
	WDTInit();

	NVICInit();

	CLKInit();

	PortInit();

//	DEBUG_UART_Init(115200);

	//100 uS = 10kHz
//	SysTimerInit(100);//TODO 100
	//50 uS = 20kHz
//	SysTimerInit(SYS_TIMER_PERIOD_US); last

//	ADCInit();
}

//void CTS0Init(void)
//{
////	PINSEL_CFG_Type PinCfg;
////	//Config P1.26 as CAP0.0
////	PinCfg.Funcnum = CTS0_FUNC;
////	PinCfg.OpenDrain = 0;
////	PinCfg.Pinmode = 0;
////	PinCfg.Portnum = CTS0_Port;
////	PinCfg.Pinnum = CTS0_bit;
////	PINSEL_ConfigPin(&PinCfg);
//
//	TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;
//	// use channel 0, CAPn.0
//	TIM_CaptureConfigStruct.CaptureChannel = 0;
//	// Enable capture on CAPn.0 rising edge
//	TIM_CaptureConfigStruct.RisingEdge = ENABLE;
//	// Enable capture on CAPn.0 falling edge
//	TIM_CaptureConfigStruct.FallingEdge = ENABLE;
//	// Generate capture interrupt
//	TIM_CaptureConfigStruct.IntOnCaption = ENABLE;
//
//	// Set configuration for Tim_config and Tim_MatchConfig
//	TIM_ConfigCapture(LPC_TIM0, &TIM_CaptureConfigStruct);
//}

//interrupt time 100uS = 10 kHz
void SysTimerInit(u32 Period_us)
{
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	TIM_MATCHCFG_Type TIM_MatchConfigStruct;

	// тактирование TIMER0 = CCLK / 4 - после reset
	// CCLK = 100 Mhz, и мы хотим получить период счета 1 uS
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_TICKVAL;
	TIM_ConfigStruct.PrescaleValue = 25; //1

	// use channel 1, MR1
	TIM_MatchConfigStruct.MatchChannel = 1;

	// Enable interrupt when MR1 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch = TRUE;

	//Enable reset on MR1: TIMER will reset if MR1 matches it
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;

	//Stop on MR1 if MR1 matches it
	TIM_MatchConfigStruct.StopOnMatch = FALSE;

	//Toggle MR0.1 pin if MR1 matches it
	TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;

	// Set Match value, count value of 100 (100 * 1uS = 100uS --> 10 kHz)
	TIM_MatchConfigStruct.MatchValue = Period_us - 1;

	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TIM_ConfigStruct);
	TIM_ConfigMatch(LPC_TIM0, &TIM_MatchConfigStruct);
//	CTS0Init();

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(TIMER0_IRQn, ((0x01 << 3) | 0x01));

	/* Enable interrupt for timer 0 */
	NVIC_EnableIRQ(TIMER0_IRQn);

	// To start timer 0
//	TIM_Cmd(LPC_TIM0, DISABLE);
	TIM_Cmd(LPC_TIM0, ENABLE);
}

void NVICInit(void)
{
	// Set Vector table offset value
	// DeInit NVIC and SCBNVIC
	NVIC_DeInit();
	NVIC_SCBDeInit();

	/* Configure the NVIC Preemption Priority Bits:
	 * two (2) bits of preemption priority, six (6) bits of sub-priority.
	 * Since the Number of Bits used for Priority Levels is five (5), so the
	 * actual bit number of sub-priority is three (3)
	 */
	NVIC_SetPriorityGrouping(0x05);

	//	NVIC_SetVTOR(0x00000000);
	//	NVIC_SetVTOR(0x00010000);
	NVIC_SetVTOR(__START_ADDRESS__);
}

void CLKInit(void)
{
	//TODO
//	SysTick_Config(SystemCoreClock / 100 - 1);	// Generate interrupt each 10 ms
//	SysTick_Config(SystemCoreClock / 1000 - 1);	// Generate interrupt each 1 ms
	SysTick_Config(SystemCoreClock / 10000 - 1);	// Generate interrupt each 0.1 ms
}

void WDTInit(void)
{
	//TODO
	// Initialize WDT, IRC OSC, interrupt mode, timeout = 2000000 microsecond
	WDT_Init(WDT_CLKSRC_IRC, WDT_MODE_RESET);
	// Start watchdog with timeout given
	WDT_Start(WDT_TIMEOUT);
}

void WDTReset(void)
{
	//TODO
	WDT_Feed();
}

void VICEnable(void)
{
	//TODO
	/* Enable PMIC interrupt level low. */
	//	CCPWrite(&PMIC.CTRL, PMIC_LOLVLEX_bm | PMIC_MEDLVLEX_bm | PMIC_HILVLEX_bm);
}

typedef enum ResetReasonMask_Enum
{
	POR_RRM = 1,
	EXTR_RRM = 2,
	WDTR_RRM = 4,
	BODR_RRM = 8,
} ResetReasonMask;

void DBGResetReason(void)
{
	char tmp[40];
	u32 rsid = LPC_SC->RSID;
	LPC_SC->RSID = 0;

	snprintf(tmp, sizeof(tmp), "Last reset: %08X ", rsid);
	msg(tmp);

	if (rsid & POR_RRM)
		msg(" POR");
	if (rsid & EXTR_RRM)
		msg(" EXT");
	if (rsid & WDTR_RRM)
		msg(" WD");
	if (rsid & BODR_RRM)
		msg(" BRN-OUT");
	msg("\n\r");
}

