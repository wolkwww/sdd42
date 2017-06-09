//adc.c

//#include <time.h>

#include <stdio.h>

#include "lpc17xx_libcfg.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_clkpwr.h"

#include "common.h"
#include "adc.h"
#include "HAL.h"
#include "UART_DEBUG.h"
#include "common.h"

volatile s32 adc_values[ADC_COUNT];

void ADCInit(void)
{
	void ConfigADCPin(u8 portnum, u8 pinnum, u8 funcnum)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = funcnum;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
		PinCfg.Pinnum = pinnum;
		PinCfg.Portnum = portnum;
		PINSEL_ConfigPin(&PinCfg);
	}
	ConfigADCPin(0, 23, 1); //AD0 OK
	ConfigADCPin(0, 24, 1); //AD1 OK cTAHO
	ConfigADCPin(0, 25, 1); //AD2 OK
	ConfigADCPin(0, 26, 1); //AD3 OK / DAC

	for (u32 ch = ADC_CHANNEL_0; ch <= ADC_CHANNEL_3; ch++)
	{
		adc_values[ch] = 50005;
	}

	ADC_Init(LPC_ADC, 13000000);

	for (u32 ch = ADC_CHANNEL_0; ch <= ADC_CHANNEL_3; ch++)
	{
		ADC_IntConfig(LPC_ADC, ch, DISABLE);
		ADC_ChannelCmd(LPC_ADC, ch, ENABLE);
	}
	//разрешаем общее прерывание по завершению всех преобразований
	ADC_IntConfig(LPC_ADC, ADC_ADGINTEN, ENABLE);

	NVIC_SetPriority(ADC_IRQn, ((0x01 << 3) | 0x01));

	//select all channels for burst conversion
	LPC_ADC->ADCR |= 0x0FF;

	ADC_BurstCmd(LPC_ADC, ENABLE);
}

static volatile bool adc_done = false;

//Main ADC Interrupt handler
void ADC_IRQHandler(void)
{
	NVIC_DisableIRQ(ADC_IRQn);

	for (u32 ch = ADC_CHANNEL_0; ch <= ADC_CHANNEL_3; ch++)
	{
		if (ADC_ChannelGetStatus(LPC_ADC, ch, ADC_DATA_DONE))
		{
			adc_values[ch] = ADC_ChannelGetData(LPC_ADC, ch);
		}
		else
		{
			adc_values[ch] = 12345;
		}
	}

	adc_done = true;
}

void ADC_Start(bool wait_for_end_conversion)
{
	adc_done = false;
	NVIC_EnableIRQ(ADC_IRQn);

	if (wait_for_end_conversion)
	{
		while (!adc_done)
			;
	}
}

void ADC_Test(void)
{
	ADC_Start(true);

	delay(100);

	for (u32 ch = ADC_CHANNEL_0; ch <= ADC_CHANNEL_3; ch++)
	{
		printf("%u\t", adc_values[ch]);
	}
	CR();
}

