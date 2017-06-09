//adc.h

#ifndef _ADC_H_
#define _ADC_H_

#include "time.h"
#include "common.h"

#define ADC_COUNT 		4
#define ADCRange 4096

extern volatile s32 adc_values[ADC_COUNT];

void ADCInit(void);
void ADC_Test(void);
void ADC_Start(bool wait_for_end_conversion);

#endif
