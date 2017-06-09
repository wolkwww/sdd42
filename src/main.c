/*
SystemCoreClock

 0123456789012345
 valu
 ��������		4
 ���� �������	2
 ��������

 * LPC_SC
 * @file	: main.c
 * @purpose	: An LPC GPS-GSM box
 * @version	: 5.0a1
 * @date	: 14.06.2010
 * @author	: mrhru
 *----------------------------------------------------------------------------
 **********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "lpc_types.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_libcfg.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"

#include "defines.h"

#include "common.h"
#include "config.h"
#include "etc.h"
#include "HAL.h"
#include "HP03M.h"
#include "init.h"
#include "lan.h"
#include "log.h"
#include "my_mem.h"
#include "ntp.h"
#include "S42init.h"
#include "test_udp.h"
#include "time.h"
#include "transport_low.h"
#include "transport.h"
#include "UART_DEBUG.h"
#include "udp.h"
#include "arp.h"
#include "tftp.h"
#include "Utils.h"
#include "sm_unit.h"

#include "SerialParallelPort.h"
#include "sbl_iap.h"
#include "CPU.h"
#include "GasMeters.h"
#include "UARTS.h"
#include "DS18B20.h"

#include "adc.h"
#include "WaterMeter_Test.h"

#include "tm_stm32f3_nrf24l01.h"
#include "DS18B20Tray.h"

int main(void)
{
	SysInit();
	STD_UART_Init();

	DBGResetReason();

	WDTReset();

//	_timezone = 0;
//	_daylight = 0;

	Wellcome();

	EndianTest();

	x(0);
	delay(100);

	msgn("HALInit");
	HALInit();

#ifdef TERMO_STATION
	TrayInit();

//	while (1)
//	{
//		TrayTest();
//		WDTReset();
//	}
	msgn("DS18B20_Init");
	DS18B20_Init();
#endif

#ifdef WATER_STATION
	WaterMeter_Init();
//	WaterMeter_Test_Loop();
#endif

//	while (1)
//	{
//		GPIO_SetValue(HR1_Port, _BV(HR1_Pin));
//		GPIO_SetValue(HR2_Port, _BV(HR2_Pin));
//		GPIO_SetValue(HR3_Port, _BV(HR3_Pin));
//		GPIO_SetValue(HR4_Port, _BV(HR4_Pin));
//
//		LED1_ON();
//		LED2_OFF();
//		delay(100);
//
//		GPIO_ClearValue(HR1_Port, _BV(HR1_Pin));
//		GPIO_ClearValue(HR2_Port, _BV(HR2_Pin));
//		GPIO_ClearValue(HR3_Port, _BV(HR3_Pin));
//		GPIO_ClearValue(HR4_Port, _BV(HR4_Pin));
//
//		LED2_ON();
//		LED1_OFF();
//		delay(100);
//	}

//serial interface for FronPanel
	SP_Init( &FrontPanelPortDef);
	SP_Init( &ValvesPortDef);
	SP_Init( &ExternValvesPortDef);
	//	SP_TestSet(&FrontPanelPortDef);
	//	SP_TestSet(&ValvesPortDef);
	//	SP_TestSet(&ExternValvesPortDef);

//	while(1)
//	{
//		GPIO_SetValue(OUTPORT_SERIAL_CLOCK_Port, _BV(OUTPORT_SERIAL_CLOCK_bit));
//		GPIO_SetValue(OUTPORT_SERIAL_DATA_Port, _BV(OUTPORT_SERIAL_DATA_bit));
//		GPIO_SetValue(OUTPORT_LATCH_CLOCK_Port, _BV(OUTPORT_LATCH_CLOCK_bit));
//		LED1_ON();
//		LED2_OFF();
//		delay(100);
//
//		GPIO_ClearValue(OUTPORT_SERIAL_CLOCK_Port, _BV(OUTPORT_SERIAL_CLOCK_bit));
//		GPIO_ClearValue(OUTPORT_SERIAL_DATA_Port, _BV(OUTPORT_SERIAL_DATA_bit));
//		GPIO_ClearValue(OUTPORT_LATCH_CLOCK_Port, _BV(OUTPORT_LATCH_CLOCK_bit));
//		LED2_ON();
//		LED1_OFF();
//		delay(100);
//	}

	// TESTS
//	SP_TestSet(&ExternValvesPortDef);	// OK

//	while(1)
//	{
//		// �������� ������ �1...�4 - OK
//		GPIO_ClearValue(M4M_Port, _BV(M4M_bit));
//		GPIO_SetValue(M4P_Port, _BV(M4P_bit));
//
//		LED1_ON();
//		delay(2000);
//
//		GPIO_ClearValue(M4P_Port, _BV(M4P_bit));
//		GPIO_SetValue(M4M_Port, _BV(M4M_bit));
//
//		LED1_OFF();
//		delay(2000);
//	}

	x(1);

#ifdef GAZ_STATION
	msgn("HP03M_Init");
	HP03M_Init();
#endif
	//#ifdef WATER_STATION
	//#endif

	ModuleInit0();

//	msgn("lan_init");
//	lan_init();

	TransportLowInit();
//	x(3);

	net_buf = (u8*) getmem(ENC28J60_MAXFRAME + 8, "ENC28J60_MAXFRAME");

	printf("net_buf = %u\n", (u32) net_buf);

	InitCPU();

//	Meters_Init();

	AddUDPHandler(12345, TestProc);
	AddUDPHandler(S42_LOCAL_PORT, S42Proc);
	AddUDPHandler(NTP_LOCAL_PORT, NTPProc);

	AddUDPHandler(TFTP_CLIENT_PORT, TFTPProc);

	u32 sender_ip = LoadBootLoaderIP();
	msgn("RESET REQUEST from ");
	prnIP(sender_ip);
	CR();
	SaveBootLoaderIP(0);

	x(4);

	for (int i = 0; i < 5; i ++)
	{
		LED1_ON();
		LED2_OFF();
		delay(100);

		LED2_ON();
		LED1_OFF();
		delay(100);
	}
	LED1_OFF();
	LED2_OFF();

	x(5);

//	ADCInit();

	bool flash = FALSE;

//	AbsoluteEnableDebugUart = FALSE;
	AbsoluteEnableDebugUart = TRUE;

	x(6);

	ModuleInit1();

	while (1)
	{
//		std_msg(0, "Hello, RS232-0!\r\n");
//		std_msg(1, "Hello, RS232-1!\r\n");
//		std_msg(2, "Hello, RS232-2!\r\n");
//		std_msg(3, "Hello, RS232-3!\r\n");
//		delay(100);
//		ADC_Test();

//		ADC_Start(TRUE);

		if (packetReceived == FALSE)
		{
//			x(7);
			LED2_OFF();
			ntp_process();
			LED2_ON();
		}

		SU_process();

#ifdef WATER_STATION
		WaterMeter_Test_Step();
#endif

#ifdef GAZ_STATION
		hp03m_process();
#endif

#ifdef TERMO_STATION
//		 very slow!!!
		DS18B20_process();
//		TrayTest();
#endif

		if (flash == TRUE)
		{
//			LED2_OFF();
			flash = FALSE;
		}
		else
		{
//			LED2_ON();
			flash = TRUE;
		}

		WDTReset();
	}

	return (1);
}

