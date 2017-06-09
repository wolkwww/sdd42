//Objects.c

#include <stdio.h>

#include "types.h"

#include "stdlib.h"
#include "string.h"

#include "Utils.h"
#include "Objects.h"


PVar CreateVar(PVarDef VarDef)
{
	PVar var = NULL;

	if (VarDef != NULL)
	{
		var = (PVar)getmem(sizeof(struct TVar), "VAR");
		
		var->CPUPinNumber = -1;
		var->VarDef = VarDef;
		
		if (VarDef->ValueSize > 0)
		{
			var->Value	= getmem(VarDef->ValueSize + 1, "VAR VALUE");
			strncpy(var->Value, VarDef->InitialValue, VarDef->ValueSize);// копирование значени€ с защитой по длине
		}
		else
		{
			var->Value	= VarDef->InitialValue;
		}
		
		printf("CREATE VAR:\t[%s], [%s]\n", VarDef->Name, VarDef->InitialValue);
	}
	
	return (var);
}

PEvent CreateEvent(PEventDef EventDef)
{
	PEvent ev = NULL;
	
	ev = getmem(sizeof(struct TEvent), "EVENT");
	ev->EventDef = EventDef;
	
	ev->Msgs = NULL;
	
	printf("CREATE EVENT:\t[%s]\n", EventDef->Name);
	
	return (ev);
}

PObject CreateModule(void)
{
	return CreateObject("MAIN", "MAIN");
}

PObject CreateObject(char* TypeName, char* ObjName)
{
	u32 i;
	PType type;
	PObject obj = getmem(sizeof(struct TObject), "OBJECT");
	
	type = FindType(TypeName);
	if (type != NULL)
	{	
		obj->Name = makestr(ObjName);
		obj->Type	= type;
		obj->InputsChanged = false;
		
		// VARS
		printf("VarCount = %d\n", (u32)obj->Type->VarCount);
		obj->Vars = getmem(sizeof(PVar) * obj->Type->VarCount, "VAR LIST");		
		for (i = 0; i < obj->Type->VarCount; i++)
		{
			obj->Vars[i] = CreateVar(&obj->Type->VarDefs[i]);
		}
		
		// EVENTS
		printf("EventCount = %d\n", (u32)obj->Type->EventCount);
		obj->Events = getmem(sizeof(PEvent) * obj->Type->EventCount, "EVENT LIST");		
		for (i = 0; i < obj->Type->EventCount; i++)
		{
			obj->Events[i] = CreateEvent(&obj->Type->EventDefs[i]);
		}
		
		// USER DATA
		if (type->UserDataSize == 0)
		{
			obj->UserData = null;
		}
		else
		{
			obj->UserData = getmem(type->UserDataSize, "USER DATA");
			printf("UserData size = %d\n", (u32)type->UserDataSize);
		}

		// запуск процедуры инициализации
		if (obj->Type->InitProc != null)
		{
			obj->Type->InitProc(obj);
		}
	}
	
	return (obj);
}

// создание из строки типа "»м€“ипа,»м€ќбъекта"
PObject CreateObject2(char* TypeAndObjName)
{
	u32 i;
	char* ObjName;
	PType	type;
	char* bufTypeName;
	PObject obj = NULL;
	
	ObjName = FindEndOfAddrItem(TypeAndObjName, ',');
	if ((ObjName != NULL) && (*ObjName == ','))
	{
		bufTypeName	= makestrbyend(TypeAndObjName, ObjName);
		ObjName++;
		printf("\t\t\t\t\t\t\tMAIN: Create Object\t|%s|%s|%s|\n",TypeAndObjName, bufTypeName, ObjName);
		type				= FindType(bufTypeName);
		
		if (type != NULL)
		{	
			obj = getmem(sizeof(struct TObject), "OBJECT(2)");
			obj->Name = makestr(ObjName);
			obj->Type	= type;
	
			// VARS
			printf("VarCount = %d\n", (u32)obj->Type->VarCount);
			obj->Vars = getmem(sizeof(PVar) * obj->Type->VarCount, "VAR LIST(2)");			
			for (i = 0; i < obj->Type->VarCount; i++)
			{
				obj->Vars[i] = CreateVar(&obj->Type->VarDefs[i]);
			}
			
			// EVENTS
			printf("EventCount = %d\n", (u32)obj->Type->EventCount);
			obj->Events = getmem(sizeof(PEvent) * obj->Type->EventCount, "EVENT LIST(2)");			
			for (i = 0; i < obj->Type->EventCount; i++)
			{
				obj->Events[i] = CreateEvent(&obj->Type->EventDefs[i]);
			}
			
			// USER DATA
			if (type->UserDataSize == 0)
			{
				obj->UserData = null;
			}
			else
			{
				obj->UserData = getmem(type->UserDataSize, "USER DATA(2)");
				printf("UserData size = %d\n", (u32)type->UserDataSize);
			}

			// запуск процедуры инициализации
			if (obj->Type->InitProc != null)
			{
				obj->Type->InitProc(obj);
			}
		}
		else
		{
			printf("Unknown type: <%s>\n", bufTypeName);
		}
		free(bufTypeName);
	}
	
	return (obj);
}

/*
<Unit2.Comp1.IN|24>
*/

