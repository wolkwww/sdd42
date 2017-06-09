//Utils.h

#ifndef _Utils_H
#define _Utils_H

#include "time.h"
#include "UnitTypes.h"

int ValueAsInt(char* val);
double ValueAsFloat(char* val);
bool StrComp(const char* s1, const char* s2);

//void FireEvent(PObject obj, char* EventName, char* param);
void FireEvent(PObject obj, const char* EventName);
void SendNextMsg(void);

PVar FindVar(PObject obj, const char* VarName);

void SetVarValue(PVar var, char* val);
void SetVarValueByName(PObject obj, char* VarName, char* val);
void SetVarValueAsInt(PObject obj, const char* VarName, s32 val);
void SetVarValueAsDouble(PObject obj, const char* VarName, double val);

void SetVarAsInt(PVar var, s32 val);
void SetVarAsFloat(PVar var, float val);
void SetVarAsDouble(PVar var, double val);


char* GetVarValue(PVar var);
char* GetObjVarValue(PObject obj, const char* VarName);
void SetEventValue(PEvent ev, char* msg);

PType FindType(char* typeName);
void AddObject(PObject object);

bool CompareName(char* start, char* stop, char* s);

void* getmem(u16 size, char* comment);
char* makestr(char* s);
char* makestrbyend(char* start, char* end);

char* FindEndOfAddrItem(char* addr, char delimiter);
u32 GetMsgIP(char* Msg);

// Часовой пояс. Для простоты забьём его константой
#define TIMEZONE    7

// Точное время (относительно момента second_count = 0)
extern volatile u32 time_offset;
extern bool IsTimeValid;

bool GetCurrDateTime(tm* ptm);

#endif

