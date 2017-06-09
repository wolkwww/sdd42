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
	
	//создаем шаблон сообщения
	PMsgTemplate pmsg = getmem(sizeof(TMsgTemplate), "MsgTemplate");
	
	// ищем разделитель адреса и параметра
	param = FindEndOfAddrItem(addr, '|');
	
	pmsg->Addresss 	= makestrbyend(addr, param);

	if (*param != '\0')
	{
		// указатель на параметр
		param++;
	}
	pmsg->Param 		= makestr(param);
	
	printf("EVENT CMD ADD addr=<%s>\tparam=<%s>\n", pmsg->Addresss, pmsg->Param);

	// ищем конец списка сообщений	
	if (ev->Msgs == NULL)
	{
		ev->Msgs = pmsg;
	}
	else
	{
		// иначе перебираем связный список
		tmpmsg = ev->Msgs;
		while(tmpmsg->next != NULL)
		{
			tmpmsg = tmpmsg->next;
		}
		// нашли
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
// присваиваем имя ноги HAL и номер ноги (в списке HAL)
// если не найдено, то -1
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
	{"SET",		VarCmdSet}, // эта команда должна быть первой, т.к. она у нее несколько упрощенных форматов для быстроты
	{"PIN",		VarCmdPin},
	{"PUSH",	VarCmdPush},

	{null,	null}			// признак конца
};

const TObjectCmdDef DefaultObjectCmdDef[] =
{
//	{"SET",		VarCmdSet}, // эта команда должна быть первой, т.к. она у нее несколько упрощенных форматов для быстроты
//	{"PIN",		VarCmdPin},
//	{"PUSH",	VarCmdPush},

	{null,	null}			// признак конца
};

