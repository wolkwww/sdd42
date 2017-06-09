// S42init.c

#include <stdio.h>

#include "stdlib.h"

#include "types.h"
#include "Init.h"
#include "Utils.h"
#include "types.h"
#include "Objects.h"
#include "Transport.h"
#include "common.h"
#include "config.h"
#include "HAL.h"

#include "MainModule.h"
#include "NetObject.h"

static void* heapstart;

void ModuleInit0(void)
{
	heapstart = getmem(8, "TEST HEAP START");
	printf("HEAP START:\t%u\n", (u32) heapstart);
	free(heapstart);

	////////////////////////////////////////////////////////////////////////////////
	printf("\n--- MAIN MODULE ---------------\n");
	MainModule = CreateModule();

	// здесь же и инициализация сети
	LocalReceiveMsgAndRun2("CREATE|NET,NET");
}

void ModuleInit1(void)
{
	if (GPIO_Read(PLAY_CONFIG_ENABLE_PORT, _BV(PLAY_CONFIG_ENABLE_PIN)) != 0)
	{
		msgn("****** PlayConfig ******");
		PlayConfig();
	}
	else
	{
		msgn("****** PlayConfig DISABLED ******");
	}

	delay(1000);

	void* heapend = getmem(8, "TEST HEAP END");
	printf("HEAP END:\t%u\n", (u32) heapend);
	free(heapend);

	printf("USED MEMORY:\t%u\n", (u32) heapend - (u32) heapstart);

	LocalReceiveMsgAndRun2("LIST");
}

