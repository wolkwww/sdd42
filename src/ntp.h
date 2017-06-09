// ntp.h

#ifndef _NTP_H
#define _NTP_H

#include "lan.h"

// Порт NTP-сервера
//#define NTP_SRV_PORT		htons(123)
#define NTP_SRV_PORT		(123)

// Локальный порт
//#define NTP_LOCAL_PORT		htons(14444)
#define NTP_LOCAL_PORT		(14444)

extern bool IsTimeValid;

void ntp_process(void);
void NTPProc(eth_frame_t* eth, u16 len);

#endif

