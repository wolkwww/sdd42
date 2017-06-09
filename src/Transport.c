//Transport.h

#include <stdio.h>

#include "types.h"

#include "UnitTypes.h"
#include "Transport.h"
#include "transport_low.h"
#include "Common.h"
#include "Utils.h"
#include "MainModule.h"
#include "udp.h"
#include "test_udp.h"

#include "string.h"
#include "stdlib.h"

struct RunnerStruct Runner;

void Run(void)
{
	PObject obj;

	// сеттеры свойств
	// это должно вызываться из команд свойствам (из дефолтной VarCmd 'SET'
//	if ((Runner.Object != null) && (Runner.SetVarProc != null))
//	{
//		Runner.SetVarProc(Runner.Object);
//	}

	// команды свойствам
	if ((Runner.Object != null) && (Runner.ObjectCmdProc != null))
	{
		Runner.ObjectCmdProc(Runner.Object, Runner.ObjectCmdProcParams);
	}

	// команды событиям
	if (Runner.EventCmdProc != null)
	{
		Runner.EventCmdProc(Runner.Event, Runner.EventCmdProcParam);
	}

	// команды свойствам
	if (Runner.VarCmdProc != null)
	{
		Runner.VarCmdProc(Runner.Object, Runner.Var, Runner.VarCmdProcParams);
	}

	if (MainModule->Type->MainProc != null)
	{
		MainModule->Type->MainProc(MainModule);
		MainModule->InputsChanged = false;
	}

	// обработка всех MainProc из списка объектов MainModule
	// TODO: ВНИМАНИЕ!!! нет вызовов внутренних подобъектов
	obj = MainModule->ObjList;
	while (obj != NULL)
	{
		if (obj->Type->MainProc != NULL)
		{
			obj->Type->MainProc(obj);
			obj->InputsChanged = false;
		}
		obj = obj->next;
	}
}

//bool TestEventCmdsAddr(char* addr, PEvent ev, char* msg)
//{
//	int i;
//	char* rdest = FindEndOfAddrItem(addr, '.');
//
//	if (addr != rdest)
//	{
//		for (i = 0; i < CMD_EVENT_COUNT; i++)
//		{
//			if (CompareName(addr, rdest, EventCmdDef[i].Name))
//			{
//				Runner.Event = ev;
//				Runner.EventCmdProc = EventCmdDef[i].Proc;
//				Runner.EventCmdProcParam = msg;
//				//if (EventCmdDef[i].Proc != NULL)
//				//{
//				//	EventCmdDef[i].Proc(ev, msg);
//				//}
//				return TRUE;
//			}
//		}
//	}
//
//	return FALSE;
//}

//bool TestEventAddr(PObject obj, char* addr, char* msg)
//{
//	char* addrend = FindEndOfAddrItem(addr, '.');
//
//	if ((addr != addrend) && (addrend))
//	{
//		for (int i = 0; i < obj->Type->EventCount; i++)
//		{
//			if (obj->Events[i] != NULL)
//			{
//				if (CompareName(addr, addrend, obj->Events[i]->EventDef->Name))
//				{
//					char* nextaddr = addrend;
//					if (*nextaddr == '.')
//					{
//						nextaddr++;
//					}
//
//					if (*nextaddr != '\0')
//					{
//						if (TestEventCmdsAddr(nextaddr, obj->Events[i], msg))
//						{
//							return TRUE;
//						}
//					}
//					break;
//				}
//			}
//		}
//	}
//
//	return FALSE;
//}

bool ProcessEventCmd(char* addr, PEvent ev, char* msg)
{
	// поиск конца команды
	char* cmdend = strchr(msg, '#');

	// формат - CMD#params
	if ((cmdend == NULL) || (msg == cmdend))
	{
		printf("OUT OF EVENT CMD : <%s>\n", msg);
		// нет команды - выходим
		return false;
	}

	// есть и имя команды и разделитель #
	if (msg != cmdend)
	{
		for (int i = 0; i < CMD_EVENT_COUNT; i ++)
		{
			if (CompareName(msg, cmdend, EventCmdDef[i].Name))
			{
				Runner.Event = ev;
				Runner.EventCmdProc = EventCmdDef[i].Proc;
				Runner.EventCmdProcParam = ++cmdend;	// остаток без разделителя '#' - это параметр команды
				return true;
			}
		}
	}

	printf("UNKNOWN EVENT CMD : <%s>\n", msg);
	return false;
}

bool TestEventAddr(PObject obj, char* addr, char* msg)
{
	for (int i = 0; i < obj->Type->EventCount; i ++)
	{
		if (obj->Events[i] != NULL)
		{
			if (StrComp(addr, obj->Events[i]->EventDef->Name))
			{
				if (ProcessEventCmd(addr, obj->Events[i], msg))
				{
					return TRUE;
				}
				break;
			}
		}
	}

	return FALSE;
}

//bool TestVarCmdsAddr(char* addr, PVar var, char* msg)
//{
//	char* addrend = FindEndOfAddrItem(addr, '.');
//
//	if (addr != addrend)
//	{
//		PVarCmdDef cmds = var->VarDef->VarCmds; // список команд из определения этой переменной
//		if (cmds == null) // если нет, то используем дефолтный список
//		{
//			cmds = DefaultVarCmdDef;
//		}
//
//		int i = 0;
//		while (cmds[i].Name != null) // for (i = 0; i < CMD_VAR_COUNT; i++)
//		{
//			if (CompareName(addr, addrend, cmds[i].Name))
//			{
//				Runner.VarCmdProc = cmds[i].Proc;
//				Runner.Var = var;
//				Runner.VarCmdProcParams = msg;
//
//				return TRUE;
//			}
//			i++;
//		}
//	}
//
//	return FALSE;
//}

// исполнение команды, передаваемой в сообщении
// общий формат msg
// CMD#param1, param2,...paramN
// вариант (присвоение, дефолтное значение для переменных)
// #param1, param2,...paramN - т.е. отсутствие имени команды
// для совместимости пока допустим и такой вариант
// param1, param2,...paramN

//PumpTimer.ON_TIME|Add#PressN.STATE|Push#PumpComp.VAL

void ProcessVarCmd(PObject obj, PVar var, char* msg)
{
	Runner.Var = var;
	Runner.Object = obj;

	// поиск конца команды
	char* cmdend = strchr(msg, '#');

	PVarCmdDef cmds = var->VarDef->VarCmds; // список команд из определения этой переменной
	if (cmds == null) // если нет, то используем дефолтный список
	{
		cmds = (PVarCmdDef) DefaultVarCmdDef;
	}

	// формат - CMD(params)
	if ((cmdend == NULL) || (msg == cmdend))
	{
		// имя пропущено (msg == cmdend) или пропущено и имя и разделитель # (cmdend == NULL)
		// дефолтная команда SET
		Runner.VarCmdProc = cmds[0].Proc;
		Runner.VarCmdProcParams = (cmdend == NULL) ? msg : ++cmdend;
		return;
	}

	// есть и имя и разделитель #

	int i = 0;
	while (cmds[i].Name != null)
	{
		if (CompareName(msg, cmdend, cmds[i].Name))
		{
			Runner.VarCmdProc = cmds[i].Proc;
			Runner.VarCmdProcParams = (cmdend == NULL) ? msg : ++cmdend;
//			printf("VAR CMD : <%s>\t\t<%s>\n", msg, Runner.VarCmdProcParams);
			return;
		}
		i ++;
	}

	printf("UNKNOWN CMD : <%s>\n", msg);

	Runner.Var = NULL;
	Runner.Object = NULL;
	Runner.VarCmdProc = NULL;
	Runner.VarCmdProcParams = NULL;
}

bool TestVarAddr(PObject obj, char* addr, char* msg)
{
	int i;
	char* addrend = FindEndOfAddrItem(addr, '.');

	if (addr != addrend)
	{
		for (i = 0; i < obj->Type->VarCount; i ++)
		{
			if (obj->Vars[i] != NULL)
			{
				if (CompareName(addr, addrend, obj->Vars[i]->VarDef->Name))
//				if (CompareName(addr, addrend, obj->Type->VarDefs[i].Name))
				{
					if ( *addrend == '\0') //последний элемент в адресе
					{
						ProcessVarCmd(obj, obj->Vars[i], msg);

//						SetVarValue(obj->Vars[i], msg);
//						obj->InputsChanged = true;

//						Runner.Object = obj;
//						Runner.SetVarProc = obj->Vars[i]->VarDef->Proc;
//
//						SetVarValue(obj->Vars[i], msg);
//
//						obj->InputsChanged = true;
						return TRUE;
					}
//					else
//					{ // какая-то команда для переменной
//						char* nextaddr = addrend;
//						if (*nextaddr == '.')
//						{
//							nextaddr++;
//						}
//
//						if (*nextaddr != '\0')
//						{
//							if (TestVarCmdsAddr(nextaddr, obj->Vars[i], msg))
//							{
//								return TRUE;
//							}
//						}
//					}
					return FALSE;
				}
			}
		}
	}

	return FALSE;
}

bool ReceiveMsgToInternalObjects(PObject obj, char* addr, char* msg);

bool TestSubObjAddr(PObject obj, char* addr, char* msg)
{
	PObject subobj = obj->ObjList;
	while (subobj != null)
	{
		if (ReceiveMsgToInternalObjects(subobj, addr, msg))
		{
			return TRUE;
		}
		subobj = subobj->next;
	}
	return FALSE;
}

void ClearRunner(void)
{
	Runner.Object = null;
	Runner.ObjectCmdProc = null;
	Runner.ObjectCmdProcParams = null;

	Runner.Event = null;
	Runner.EventCmdProc = null;
	Runner.EventCmdProcParam = null;

	Runner.VarCmdProc = null;
	Runner.Var = null;
	Runner.VarCmdProcParams = null;
}

void ProcessObjectCmd(PObject obj, char* msg)
{
	Runner.Object = obj;

	// поиск конца команды
	char* cmdend = strchr(msg, '#');

	PObjectCmdDef cmds = obj->Type->ObjCmds; // список команд из определения этого объекта
	if (cmds == null) // если нет, то используем дефолтный список
	{
		cmds = (PObjectCmdDef) DefaultObjectCmdDef;
	}

	// формат - CMD(params)
	if ((cmdend == null) || (msg == cmdend))
	{
		// имя пропущено (msg == cmdend) или пропущено и имя и разделитель # (cmdend == NULL)
		// дефолтная команда SET
		Runner.ObjectCmdProc = cmds[0].Proc;
		Runner.ObjectCmdProcParams = (cmdend == null) ? msg : ++cmdend;
		return;
	}

	// есть и имя и разделитель #

	int i = 0;
	while (cmds[i].Name != null)
	{
		if (CompareName(msg, cmdend, cmds[i].Name))
		{
			Runner.ObjectCmdProc = cmds[i].Proc;
			Runner.ObjectCmdProcParams = (cmdend == NULL) ? msg : ++cmdend;
//			printf("VAR CMD : <%s>\t\t<%s>\n", msg, Runner.VarCmdProcParams);
			return;
		}
		i ++;
	}

	printf("UNKNOWN OBJECT CMD : <%s>\n", msg);

	Runner.Object = NULL;
	Runner.ObjectCmdProc = NULL;
	Runner.ObjectCmdProcParams = NULL;
}

bool ReceiveMsgToInternalObjects(PObject obj, char* addr, char* msg)
{
	char* addrend;
	char* nextaddr;

	addrend = FindEndOfAddrItem(addr, '.');
	if (addr != addrend)
	{
		// это нам?
		if ((obj == MainModule) || CompareName(addr, addrend, obj->Name))
		{
			if (obj == MainModule)
			{
				addrend = addr;
			}

			nextaddr = addrend;
			if ( *nextaddr == '.')
			{
				nextaddr ++;
			}

			if ( *nextaddr != '\0') // продолжение адреса следует
			{
				if (TestSubObjAddr(obj, nextaddr, msg)) //TODO
				{
					return TRUE;
				}

				if (TestVarAddr(obj, nextaddr, msg)) //TODO
				{
					return TRUE;
				}

				if (TestEventAddr(obj, nextaddr, msg)) //TODO
				{
					return TRUE;
				}

				printf("\t\t\t\t\tUnknown address: =[%s]=\n", addr);
				return FALSE;
			}
			else	// адрес закончился, проверяем на команды
			{
				ProcessObjectCmd(obj, msg);
			}
		}
	}
	return FALSE;
}

void ReceiveMsg(char* addr, char* msg)
{
//	printf("ReceiveMsg:\t<%s/%s>\n", addr, msg);
	ReceiveMsgToInternalObjects(MainModule, addr, msg);
}

void LocalReceiveMsgAndRun(char* addr, char* msg)
{
	ClearRunner();

	ReceiveMsg(addr, msg);

	// UNITS RUNNER
	Run();

	// SENDER
	SendNextMsg();
}

void LocalReceiveMsgAndRun2(char* AddrMsg)
{
	strcpy(MsgBuf, AddrMsg);

	ClearRunner();

	ReceiveFullMsg(MsgBuf);
	//ReceiveMsg(addr, msg);

	// UNITS RUNNER
//	msgn(AddrMsg);
	Run();

	// SENDER
	SendNextMsg();
}

void ReceiveFullMsg(char* AddrMsg)
{
	char* addr = AddrMsg;
	char* msg = strchr(AddrMsg, '|');

	if (msg == NULL)
	{
		msg = AddrMsg + strlen(AddrMsg); // points to final \0
	}
	else
	{
		*msg = '\0';
		msg ++;
	}
	ReceiveMsg(addr, msg);
}

bool IsInternalMsg(char* msg)
{
	char* rdest = FindEndOfAddrItem(msg, '.');

	return (CompareName(msg, rdest, MainModule->Name));
}

u8 SendMsg(char* addr, char* msg);

void SendMsg2Param(char* addr, char* param1, char* param2)
{

	int len1 = strlen(param1);
	int len2 = strlen(param2);
	char* msg = getmem(len1 + len2 + 1, "SendMsg2Param");
	strcpy(msg, param1);
	strcpy(msg + len1, param2);
	SendMsg(addr, msg);
	free(msg);
//*/
}

// вызывается из команды Push
void SendPushMsg(PVar var, char* addr)
{
	// это инициирует чтение из ножки HAL'а, если она указана
	GetVarValue(var);

//	printf("SendPushMsg: <%s> <%s>\n", addr, var->Value);
	if (SendMsg(addr, var->Value) != 0)
	{
		//push_cmd_cnt++;
	}
}

u8 SendMsg(char* addr, char* msg)
{
	u32 ip = GetMsgIP(addr);
	if (ip != 0)
	{
		// обрезанный адрес, без IP
		char* addrcut = strchr(addr, ':') + 1;
		sprintf(MsgBuf, "%s|%s", addrcut, msg);
		return UdpSendMsg(ip, MsgBuf);
	}
	else
	{
		//loopback
		sprintf(MsgBuf, "%s|%s", addr, msg);
		packetReceived = TRUE;

		return 1;
//		ReceiveMsg(addr, msg);
	}
}

void SendErrorMsg(u32 ip, char* addr, char* msg)
{
	if (ip != 0)
	{
		sprintf(MsgBuf, "%s|%s", addr, msg);
		UdpSendMsg(ip, MsgBuf);
	}
	else
	{
		printf("**************************************************************ERROR:\t<%s>\t<%s>\n", addr, msg);
	}
}
