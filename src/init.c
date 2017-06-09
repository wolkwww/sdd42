// init.c

#include "types.h"
#include "init.h"
#include "common.h"
#include "HAL.h"


/*
	Этих значений нет ни в одном заголовочном файле, взяты из Standard Peripherial Library
*/
#define CLK_SWR_HSE 0xB4
#define TIM4_PRESCALER_128 ((u8)0x07)

//#define LED0_PORT           			GPIOD
//#define LED0_PIN                 	GPIO_PIN_0                  // PD0
//
//#define LED_RED_PORT           		GPIOB
//#define LED_RED_PIN               GPIO_PIN_5
//
//#define LED_GREEN_PORT       			GPIOB
//#define LED_GREEN_PIN             GPIO_PIN_3
//
//#define LED_YELLOW_PORT						GPIOB
//#define LED_YELLOW_PIN            GPIO_PIN_1
//
//#define SW1_PORT           				GPIOB
//#define SW1__PIN                 	GPIO_PIN_6


//void _led(u8 port, u8 pin, u8 state)
//{
//	switch(state)
//	{
//		case OFF:
//			GPIO_Set(port, pin);
//			break;
//		case ON:
//			GPIO_Reset(port, pin);
//			break;
//		case SW:
//			GPIO_Invert(port, pin);
//			break;
//	}
//}
//
void RLED(u8 state)
{
//	_led(LED_RED_PORT, LED_RED_PIN, state);
}

void GLED(u8 state)
{
//	_led(LED_GREEN_PORT, LED_GREEN_PIN, state);
}

void YLED(u8 state)
{
//	_led(LED_YELLOW_PORT, LED_YELLOW_PIN, state);
}

void LED0_On(void)
{
//	GPIO_WriteLow(LED0_PORT, LED0_PIN);
}

void LED0_Off(void)
{
//	GPIO_WriteHigh(LED0_PORT, LED0_PIN);
}

void LED0_Switch(void)
{
//	GPIO_WriteReverse(LED0_PORT, LED0_PIN);
}

/*
	Инициализация периферии
*/

void InitGPIO(void)
{
//	GPIO_Init(LED_RED_PORT, 		LED_RED_PIN, 		GPIO_MODE_OUT_PP_HIGH_SLOW);
//	GPIO_Init(LED_GREEN_PORT, 	LED_GREEN_PIN, 	GPIO_MODE_OUT_PP_HIGH_SLOW);
//	GPIO_Init(LED_YELLOW_PORT,	LED_YELLOW_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);
//	GPIO_Init(LED0_PORT, 				LED0_PIN, 			GPIO_MODE_OUT_PP_HIGH_SLOW);

	GPIO_Init_Input_PullUp(PLAY_CONFIG_ENABLE_PORT, PLAY_CONFIG_ENABLE_PIN);
}

#define CCR1_Val  ((u16)32767)
#define CCR2_Val  ((u16)16383)
#define CCR3_Val  ((u16)8191)


//static void InitTIM1(void)
//{
//	/* TIM1 Peripheral Configuration */
//	TIM1_DeInit();
//
//	/* Time Base configuration */
//				/*
//	TIM1_Prescaler = 0
//	TIM1_CounterMode = TIM1_COUNTERMODE_UP
//	TIM1_Period = 65535
//	TIM1_RepetitionCounter = 0
//				*/
//
//	TIM1_TimeBaseInit(0, TIM1_COUNTERMODE_UP, 65535,0);
//
//	/* Channel 1, 2 and 3 Configuration in PWM mode */
//	/*
//				TIM1_OCMode = TIM1_OCMODE_PWM2
//	TIM1_OutputState = TIM1_OUTPUTSTATE_ENABLE
//	TIM1_OutputNState = TIM1_OUTPUTNSTATE_ENABLE
//	TIM1_Pulse = CCR1_Val
//	TIM1_OCPolarity = TIM1_OCPOLARITY_LOW
//	TIM1_OCNPolarity = TIM1_OCNPOLARITY_LOW
//	TIM1_OCIdleState = TIM1_OCIDLESTATE_SET
//	TIM1_OCNIdleState = TIM1_OCIDLESTATE_RESET
//				*/
//	TIM1_OC1Init(TIM1_OCMODE_PWM2, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_ENABLE,  CCR1_Val, TIM1_OCPOLARITY_LOW, TIM1_OCNPOLARITY_LOW, TIM1_OCIDLESTATE_SET,  TIM1_OCNIDLESTATE_RESET);
//
//	/* TIM1_Pulse = CCR2_Val */
//
//				TIM1_OC2Init(TIM1_OCMODE_PWM2, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_ENABLE,  CCR2_Val, TIM1_OCPOLARITY_LOW, TIM1_OCNPOLARITY_LOW, TIM1_OCIDLESTATE_SET,  TIM1_OCNIDLESTATE_RESET);
//
//	/* TIM1_Pulse = CCR3_Val */
//
//	TIM1_OC3Init(TIM1_OCMODE_PWM2, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_ENABLE,  CCR3_Val, TIM1_OCPOLARITY_LOW, TIM1_OCNPOLARITY_LOW, TIM1_OCIDLESTATE_SET,  TIM1_OCNIDLESTATE_RESET);
//
//	/* Automatic Output enable, Break, dead time and lock configuration*/
//	/*
//				TIM1_OSSIState = TIM1_OSSISTATE_ENABLE
//	TIM1_LockLevel = TIM1_LOCKLEVEL_1
//	TIM1_DeadTime = 117
//	TIM1_Break = TIM1_BREAK_ENABLE
//	TIM1_BreakPolarity = TIM1_BREAKPOLARITY_HIGH
//	TIM1_AutomaticOutput = TIM1_AUTOMATICOUTPUT_ENABLE
//				*/
//				TIM1_BDTRConfig( TIM1_OSSISTATE_ENABLE,  TIM1_LOCKLEVEL_1, 117, TIM1_BREAK_ENABLE,  TIM1_BREAKPOLARITY_HIGH, TIM1_AUTOMATICOUTPUT_ENABLE);
//
//	/* TIM1 counter enable */
//	TIM1_Cmd(ENABLE);
//
//	/* Main Output Enable */
//	TIM1_CtrlPWMOutputs(ENABLE);
//}
//
//
//#define TIM4_PERIOD       124
//
//static void InitTIM4(void)
//{
//  /* TIM4 configuration:
//   - TIM4CLK is set to 16 MHz, the TIM4 Prescaler is equal to 128 so the TIM1 counter
//   clock used is 16 MHz / 128 = 125 000 Hz
//  - With 125 000 Hz we can generate time base:
//      max time base is 2.048 ms if TIM4_PERIOD = 255 --> (255 + 1) / 125000 = 2.048 ms
//      min time base is 0.016 ms if TIM4_PERIOD = 1   --> (  1 + 1) / 125000 = 0.016 ms
//  - In this example we need to generate a time base equal to 1 ms
//   so TIM4_PERIOD = (0.001 * 125000 - 1) = 124 */
//
//  /* Time base configuration */
//  TIM4_TimeBaseInit(TIM4_PRESCALER_128, TIM4_PERIOD);
//  /* Clear TIM4 update flag */
//  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
//  /* Enable update interrupt */
//  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
//
//  /* enable interrupts */
//  enableInterrupts();
//
//  /* Enable TIM4 */
//  TIM4_Cmd(ENABLE);
//}

void HALInit(void)
{	
//	CLK_DeInit ();
	// external 16 MHz

//	CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
//	CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE, ENABLE);


	
	/*
		Переключаемся на внешний кварц (HSE)
	CLK->CKDIVR = 0;	// После этого начинаем работать от встроенного генератора на 16 МГц
	CLK->SWCR |= CLK_SWCR_SWEN; // Начинаем переключение
	CLK->SWR = CLK_SWR_HSE; // Инициируем переключение на HSE
	// Никого не ждём, переключится, как только сможет
	*/
	

	/*
		Светодиоды
	*/
//	Pin_Out(LED1);	
//	Pin_Out(LED2);		


//  FLASH_Unlock (FLASH_MEMTYPE_DATA); 			// Разблокировать область данных FLASH.
//  FLASH_ProgramOptionByte(OPT2, 1<<AFR7); // Разрешить beep в регистре OPT2.

	InitGPIO();

//	InitTIM4();

	//InitTIM1();

//	InitUart();
}


