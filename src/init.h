// init.h

#ifndef _INIT_H
#define _INIT_H

#include "types.h"

#define PLAY_CONFIG_ENABLE_PORT		(2)
#define PLAY_CONFIG_ENABLE_PIN		(12)

void HALInit(void);

void LED0_On(void);
void LED0_Off(void);
void LED0_Switch(void);

#define	OFF												0
#define	ON												1
#define	SW												2

void RLED(u8 state);
void GLED(u8 state);
void YLED(u8 state);

#endif
