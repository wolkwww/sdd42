//Objects.h

#ifndef _Objects_H
#define _Objects_H

#include "UnitTypes.h"

PVar CreateVar(PVarDef VarDef);
PEvent CreateEvent(PEventDef EventDef);

PObject CreateObject(char* TypeName, char* ObjName);
PObject CreateObject2(char* TypeAndObjName);

PObject CreateModule(void);

#endif


