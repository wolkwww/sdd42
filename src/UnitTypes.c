// UnitTypes.c

#include <stdio.h>

#include "types.h"
#include "Utils.h"
#include "UnitTypes.h"
#include "Transport.h"
#include "CPU.h"

// EVENT CMDs //////////////////////////////////////////////////////////////////

void EventCmdAdd(PEvent ev, char* addr)
{
	PMsgTemplate tmpmsg;
	char* param;
	
	//������� ������ ���������
	PMsgTemplate pmsg = getmem(sizeof(TMsgTemplate), "MsgTemplate");
	
	// ���� ����������� ������ � ���������
	param = FindEndOfAddrItem(addr, '|');
	
	pmsg->Addresss 	= makestrbyend(addr, param);

	if (*param != '\0')
	{
		// ��������� �� ��������
		param++;
	}
	pmsg->Param 		= makestr(param);
	
	printf("EVENT CMD ADD addr=<%s>\tparam=<%s>\n", pmsg->Addresss, pmsg->Param);

	// ���� ����� ������ ���������	
	if (ev->Msgs == NULL)
	{
		ev->Msgs = pmsg;
	}
	else
	{
		// ����� ���������� ������� ������
		tmpmsg = ev->Msgs;
		while(tmpmsg->next != NULL)
		{
			tmpmsg = tmpmsg->next;
		}
		// �����
		tmpmsg->next = pmsg;
	}
}

void EventCmdDel(PEvent ev, char* msg)
{
	//TODO OR NOT TODO:
}

void EventCmdClear(PEvent ev, char* msg)
{
	//TODO OR NOT TODO:
}

const TEventCmdDef EventCmdDef[CMD_EVENT_COUNT] = 
{
	{"ADD",		EventCmdAdd},
	{"DEL",		EventCmdDel},
	{"CLEAR",	EventCmdClear}
};

// VAR CMDs ////////////////////////////////////////////////////////////////////
// ����������� ��� ���� HAL � ����� ���� (� ������ HAL)
// ���� �� �������, �� -1
void VarCmdPin(PObject obj, PVar var, char* param)
{
//	var->CPUPinName = makestr(param);
//	var->CPUPinNumber = FindCPUPinNumber(var->CPUPinName);
	var->CPUPinNumber = FindCPUPinNumber(param);

	printf("VAR CMD PIN : <%s> := [%d].[%d] \n", param, var->CPUPinNumber, (&CPUPins[var->CPUPinNumber])->PinNumber);
}

// Mod1.Item.Var1.Push|Addr1
void VarCmdPush(PObject obj, PVar var, char* param)
{
//	printf("VAR CMD PUSH: <%s>\n", param);
	SendPushMsg(var, param);
}

void VarCmdSet(PObject obj, PVar var, char* param)
{
//	printf("VAR CMD SET <%s.%s>: <%s>\n", obj->Name, var->VarDef->Name, param);
	SetVarValue(var, param);
	if (var->VarDef->Proc != null)
	{
		var->VarDef->Proc(obj, var);
	}
	obj->InputsChanged = true;
}

const TVarCmdDef DefaultVarCmdDef[] =
{
	{"SET",		VarCmdSet}, // ��� ������� ������ ���� ������, �.�. ��� � ��� ��������� ���������� �������� ��� ��������
	{"PIN",		VarCmdPin},
	{"PUSH",	VarCmdPush},

	{null,	null}			// ������� �����
};

const TObjectCmdDef DefaultObjectCmdDef[] =
{
//	{"SET",		VarCmdSet}, // ��� ������� ������ ���� ������, �.�. ��� � ��� ��������� ���������� �������� ��� ��������
//	{"PIN",		VarCmdPin},
//	{"PUSH",	VarCmdPush},

	{null,	null}			// ������� �����
};

