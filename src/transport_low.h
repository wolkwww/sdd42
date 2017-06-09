// transport_low.h

#ifndef _Transport_Low_H
#define _Transport_Low_H

#include "types.h"
#include "lan.h"

void TransportLowInit(void);
void TransportSendMsg(char* MsgBuf);	
void RFSendMsg(char* addr, char* msg);	

bool processReceivedPackets(void);
void S42Proc(eth_frame_t* eth, u16 len);

#endif
