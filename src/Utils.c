// Utils.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "UnitTypes.h"
#include "Transport.h"
#include "CPU.h"
#include "Common.h"
#include "S42Common.h"
#include "MainModule.h"
#include "lan.h"
#include "HAL.h"

#include "Utils.h"

//#include "micro-common.h"
//#include "STM32W108XX_micro-common.h"

s32 ValueAsInt(char* val)
{
	if (val != null )
	{
//		printf("\n\nValueAsInt = <%s>\n\n", val);
		return (atoi(val));
	}
	else
	{
		return 0;
	}
}

double ValueAsFloat(char* val)
{
	if (val != null )
	{
		char* pEnd;
		double res;
		res = strtod(val, &pEnd);
		return (val != pEnd) ? res : 0.0;
	}
	else
	{
		return 0.0;
	}
}

bool StrComp(const char* s1, const char* s2)
{
	return (bool)(strcmp(s1, s2) == 0);
}

PType FindType(char* typeName)
{
	int i = 0;
	PType res = NULL;

	while (Types[i] != null )
	{
		if (StrComp(Types[i]->Name, typeName))
		{
			res = Types[i];
			break;
		}

		i++;
	}

	return res;
}

PEvent FindEvent(PObject obj, const char* EventName)
{
	int i;
	PEvent ev = NULL;

	for (i = 0; i < obj->Type->EventCount; i++)
	{
		if (obj->Events[i] != NULL)
		{
			if (StrComp(EventName, obj->Events[i]->EventDef->Name))
			{
				ev = obj->Events[i];
				break;
			}
		}
	}

	return ev;
}

struct FireEventStruct
{
	u32 PsTimeSet; //идет опережающе
	//u32 PsTimeGet; //идет догон€юще

	PObject Object;
	PEvent CurrentEventToSend;
	PMsgTemplate FirstMsgTemplateToSend;
} FES =
{ 0, 0, null, null };

//void FireEvent(PObject obj, char* EventName, char* param)
void FireEvent(PObject obj, const char* EventName)
{
//	int i;
	PEvent ev;
//	PMsgTemplate pmsg;

//	printf("FIRE EVENT:\t%s.%s\n", obj->Name, EventName);

	ev = FindEvent(obj, EventName);

	if (ev != NULL)
	{
		// помечаем, если список сообщений не пуст
		if (ev->Msgs != null )
		{
			FES.PsTimeSet++;
			if (FES.PsTimeSet == 0)
			{
				FES.PsTimeSet = 1;
			}

			ev->PsTime = FES.PsTimeSet;
		}
	}
}

void FindNextMessageToSend(void)
{
//	PEvent	ObjEvent;
	PObject obj;
	int i;
	u32 minPsTime;

	if (FES.FirstMsgTemplateToSend != null )
	{
		FES.FirstMsgTemplateToSend = FES.FirstMsgTemplateToSend->next;
		if (FES.FirstMsgTemplateToSend != null )
		{
			return;
		}
		else
		{
			//с этим событием закончили
			FES.CurrentEventToSend->PsTime = 0;

			FES.Object = null;
			FES.CurrentEventToSend = null;
			FES.FirstMsgTemplateToSend = null;
		}
	}

	// ищем следующее событие

	minPsTime = 0xFFFFFFFF;
	// перебор объектов
	obj = MainModule->ObjList;
	// указатель на Event c минимальным PsTime, большим или равным FES.PsTimeGet
	FES.CurrentEventToSend = null;

	while (obj != null )
	{
		// перебор событий в объекте
		for (i = 0; i < obj->Type->EventCount; i++)
		{
			PEvent ObjEvent = obj->Events[i];
			if ((ObjEvent->PsTime != 0) && (ObjEvent->PsTime < minPsTime))
			{
				minPsTime = ObjEvent->PsTime;
				// это наше очередное новое "минимальное" событие
				FES.Object = obj;
				FES.CurrentEventToSend = ObjEvent;
				FES.FirstMsgTemplateToSend = ObjEvent->Msgs;
			}
		}

		// к следующему объекту
		obj = obj->next;
	}

	if ((FES.CurrentEventToSend != null )&& (FES.FirstMsgTemplateToSend == null)){
	FES.CurrentEventToSend->PsTime = 0;

	FES.Object = null;
	FES.CurrentEventToSend = null;
	FES.FirstMsgTemplateToSend = null;
}
}

void SendMsgFromMsgTemplate(PObject obj, PMsgTemplate MsgTemplate)
{
	char* param = MsgTemplate->Param;
	if (param == null )
	{
		param = "";
	}
	else
	{
		if (param[0] == '%') // var name
		{
			param = GetObjVarValue(obj, &(param[1]));
		}
	}
//	printf("SEND MSG <%s|%s>\n\r", MsgTemplate->Addresss, param);
	SendMsg(MsgTemplate->Addresss, param);
}

//SENDER
void SendNextMsg(void)
{
	// поскольку дл€ приема и передачи сообщений используетс€
	// один буфер, то об€зательна проверка (packetReceived != TRUE),
	// так как после приема и обработки сообщений Run() в цикле
	//   void SU_process(void)
	//	   ...
	//	   // RECEIVER
	//	   processReceivedPackets();
	// 	   // UNITS RUNNER
	//     Run();  // <- вот здесь
	// возможно по€вление сообщени€ дл€ отправки, например командой PUSH.
	// »наче, возможно "затирание" этого сообщени€.

	if (packetReceived != TRUE)
	{
		if (FES.FirstMsgTemplateToSend == null )
		{
			FindNextMessageToSend();
		}

		if (FES.FirstMsgTemplateToSend != null )
		{
			SendMsgFromMsgTemplate(FES.Object, FES.FirstMsgTemplateToSend);
			FindNextMessageToSend();
		}
	}
}

/*
 void FireEvent(PObject obj, char* EventName, char* param)
 {
 int i;
 PEvent ev;
 PMsgTemplate pmsg;

 printf("FIRE EVENT:\t%s.%s (%s)\n", obj->Name, EventName, param);

 ev = FindEvent(obj, EventName);
 if (ev != NULL)
 {
 pmsg = ev->Msgs;
 while(pmsg != NULL)
 {
 SendMsg(pmsg->Addresss, param);
 pmsg = pmsg->next;
 }
 }
 }
 */

PVar FindVar(PObject obj, const char* VarName)
{
	int i;
	PVar var = NULL;

	for (i = 0; i < obj->Type->VarCount; i++)
	{
		if (obj->Vars[i] != NULL)
		{
			if (StrComp(VarName, obj->Vars[i]->VarDef->Name))
			{
				var = obj->Vars[i];
				break;
			}
		}
	}

	return var;
}

void SetEventValue(PEvent ev, char* msg)
{
	if (ev != NULL)
	{
		//TODO:
	}
}

void SetVarValue(PVar var, char* val)
{
	u8 len;
	if (var != NULL)
	{
		len = var->VarDef->ValueSize;
		if (len == 0)
		{
			// при нулевом размере - просто храним ссылку
			var->Value = val;
		}
		else
		{
			strncpy(var->Value, val, len);
		}

//		if (var->CPUPinName != NULL)
//		{
//			SetCPUPinValue(var->CPUPinName, ValueAsInt(val));
//		}
		if (var->CPUPinNumber != -1)
		{
			SetCPUPinValue(var->CPUPinNumber, ValueAsInt(val));
		}
	}
}

void SetVarValueByName(PObject obj, char* VarName, char* val)
{
	SetVarValue(FindVar(obj, VarName), val);
}

void SetVarValueAsInt(PObject obj, const char* VarName, s32 val)
{
	SetVarAsInt(FindVar(obj, VarName), val);
}

void SetVarValueAsDouble(PObject obj, const char* VarName, double val)
{
	SetVarAsDouble(FindVar(obj, VarName), val);
}

static char conv_buf[32];

void SetVarAsInt(PVar var, s32 val)
{
	sprintf(conv_buf, "%d", val);
	SetVarValue(var, conv_buf);
}

void SetVarAsFloat(PVar var, float val)
{
	sprintf(conv_buf, "%.3f", val); //TODO sprintf не поддерживает float
	SetVarValue(var, conv_buf);
}

void SetVarAsDouble(PVar var, double val)
{
//	printf("%f\n\r", val);
	sprintf(conv_buf, "%.3f", val); //TODO sprintf не поддерживает float
	SetVarValue(var, conv_buf);
}

//char* GetVarValue(PVar var)
//{
//	if (var->CPUPinName != NULL)
//	{
//		PCPUPin pin = FindCPUPin(var->CPUPinName);
//		if ((pin != NULL) && (pin->CPUPin_Get != NULL))
//		{
//			u32 pinval = pin->CPUPin_Get(pin);
//			sprintf(var->Value, "%d", pinval);
//		}
//	}
//	return (var->Value);
//}

char* GetVarValue(PVar var)
{
	if (var->CPUPinNumber != -1)
	{
		PCPUPin pin = &CPUPins[var->CPUPinNumber];
		if ((pin != NULL ) && (pin->CPUPin_Get != NULL ))
		{
//			s32 pinval = pin->CPUPin_Get(pin, var->Value, var->VarDef->ValueSize);
//			sprintf(var->Value, "%d", pinval);

			pin->CPUPin_Get(pin, var->Value, var->VarDef->ValueSize);
		}
	}
	return (var->Value);
}

char* GetObjVarValue(PObject obj, const char* VarName)
{
	PVar var = FindVar(obj, VarName);

	if (var != NULL)
	{
		return GetVarValue(var);
	}
	else
	{
		return NULL ;
	}
}

void AddObject(PObject object)
{
}

void* getmem(u16 size, char* comment)
{
	void* mem = NULL;
	if (size != 0)
	{
		mem = (void*) malloc(size);
		if (mem != null )
		{
			memset(mem, 0, size);
//			printf("GETMEM:\t&%d[%d]\t(%s)\n", (u32) mem, (u32) size, comment);
		}
		else
		{
			sprintf(MsgBuf, "GETMEM: OUT OF MEMORY! size=[%d]\t(%s)\n", (u32) size, comment);
			SendErrorMsg(server_ip, "ERROR", MsgBuf);
			printf("%s", MsgBuf);
		}
	}
	return (mem);
}

char* makestr(char* s)
{
	u16 size = 0;
	char* buf = NULL;

	if (s[0] != '\0') // strlen > 0
	{
		size = strlen(s) + 1;
		buf = getmem(size, "makestr");
		strcpy(buf, s);
		printf("STR:\t%s\t(%d)\n", s, (u32) size);
	}

	return buf;
}

// создание строки по указател€м на начало (включительно) и конец (исключительно)
//     012345678901234567890
// т.е THIS IS STRING, MY STRING
//     ^             ^
//   start          end
// результатом будет новый буфер с содержанием "THIS IS STRING\0"
char* makestrbyend(char* start, char* end)
{
	char* buf = NULL;
	unsigned int size = end - start;

	if (size > 0)
	{
		buf = getmem(size + 1, "makestrbyend");
		strncpy(buf, start, size);
		buf[size] = '\0';
		printf("STR:\t%s\t(%d)\n", buf, (u32) size);
	}

	return buf;
}

char* FindEndOfAddrItem(char* addr, char delimiter)
{
	char* rdest = strchr(addr, delimiter);

	if (rdest == NULL)
	{
		rdest = addr + strlen(addr); // points to final \0
	}

	return rdest;
}

u32 GetMsgIP(char* Msg)
{
	u32 ip = 0;
	int a, b, c, d;

	if (sscanf(Msg, "%d.%d.%d.%d:", &a, &b, &c, &d) > 0)
	{
		ip = inet_addr(a, b, c, d);
	}

	return (ip);
}

// “очное врем€ (относительно момента second_count = 0)
volatile u32 time_offset;
bool IsTimeValid = TRUE;

bool GetCurrDateTime(tm* ptm)
{
	if (IsTimeValid)
	{
		u32 loctime;
		// ¬ычисл€ем врем€
		loctime = time_offset + GetRTC() + 60UL * 60 * TIMEZONE;
		if (gmtime_x((const time_t*) (&loctime), ptm) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// compare string between *start and *stop with s
bool CompareName(char* start, char* stop, char* s)
{
	int len = stop - start;
	while ((len-- > 0) && (*start++ == *s++))
		;

	return (bool)((len == -1) && (*s == '\0'));
}
