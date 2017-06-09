//Transport.h

#ifndef _Transport_H
#define _Transport_H

#include "UnitTypes.h"

extern volatile bool packetReceived;

void ClearRunner(void);

void ReceiveMsg(char* Addr, char* msg);
u8 SendMsg(char* Addr, char* msg);
void ReceiveFullMsg(char* AddrMsg);
void SendMsg2Param(char* addr, char* param1, char* param2);
bool IsInternalMsg(char* msg);
void SendPushMsg(PVar var, char* addr);
void SendErrorMsg(u32 ip, char* addr, char* msg);

void LocalReceiveMsgAndRun(char* addr, char* msg);
void LocalReceiveMsgAndRun2(char* AddrMsg);

struct RunnerStruct
{
	PObject			Object;
	TObjectCmdProc	ObjectCmdProc;
	char* 			ObjectCmdProcParams;

	PEvent			Event;
	TEventCmdProc	EventCmdProc;
	char*			EventCmdProcParam;
	
	TVarCmdProc		VarCmdProc;
	PVar 			Var;
	char* 			VarCmdProcParams;
};

void Run(void);

#endif
