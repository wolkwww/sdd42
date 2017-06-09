/***********************************************************************//**
 * @file	: lpc17xx_libcfg.h
 * @purpose	: Library configuration file
 * @version	: 1.0
 * @date	: 26. Mar. 2009
 * @author	: HieuNguyen
 **************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **************************************************************************/

#ifndef LPC17XX_LIBCFG_H_
#define LPC17XX_LIBCFG_H_

#include "lpc_types.h"

#ifndef __GNUC__
#define   __GNUC__		4
#endif

//#undef __BUILD_WITH_EXAMPLE__
#define __BUILD_WITH_EXAMPLE__

/************************** DEBUG MODE DEFINITIONS *********************************/
/* Un-comment the line below to compile the library in DEBUG mode, this will expanse
 the "CHECK_PARAM" macro in the FW library code */

//#define DEBUG    1

/******************* PERIPHERAL FW LIBRARY CONFIGURATION DEFINITIONS ***********************/

/* Comment the line below to disable the specific peripheral inclusion */

/* GPIO ------------------------------- */
#define _GPIO

/* UART ------------------------------- */
#define _UART 1
//#define _UART0 1
#define _UART1 1
#define _UART2 1
#define _UART3 1

/* TIMER ------------------------------- */
#define _TIM 1

/* 	ADC------------------------------- */
#define _ADC 1

/*  SSP------------------------------- */
#define _SSP 1

/*  SPI------------------------------- */
#define _SPI

/*  WDT------------------------------- */
#define _WDT 1

/*  DAC------------------------------- */
#define _DAC 1

/*  PWM------------------------------- */
#define _PWM 1

/************************** GLOBAL/PUBLIC MACRO DEFINITIONS *********************************/

#ifdef  DEBUG
/*******************************************************************************
 * @brief		The CHECK_PARAM macro is used for function's parameters check.
 * 				It is used only if the library is compiled in DEBUG mode.
 * @param[in]	expr - If expr is false, it calls check_failed() function
 *                    	which reports the name of the source file and the source
 *                    	line number of the call that failed.
 *                    - If expr is true, it returns no value.
 * @return		None
 *******************************************************************************/
#define CHECK_PARAM(expr) ((expr) ? (void)0 : check_failed((uint8_t *)__FILE__, __LINE__))
#else
#define CHECK_PARAM(expr)
#endif /* DEBUG */

/************************** GLOBAL/PUBLIC FUNCTION DECLARATION *********************************/

#ifdef  DEBUG
void check_failed(u8 *file, u32 line);
#endif

#endif /* LPC17XX_LIBCFG_H_ */
