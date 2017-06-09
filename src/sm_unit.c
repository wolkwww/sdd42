// sm_unit.c

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "types.h"

#include "Init.h"
#include "s42init.h"
#include "Transport.h"
#include "transport_low.h"
#include "Utils.h"
#include "transport_low.h"

void SU_process(void)
{
	ClearRunner();

	// RECEIVER
	processReceivedPackets();

	// UNITS RUNNER
	Run();

	// SENDER
	SendNextMsg();
}

