// UnitTypes.h

#ifndef _UnitTypes_H
#define _UnitTypes_H

#include "types.h"

// Type			/////////////////////////////////////////////////////////////

//typedef struct TType;
//typedef	const struct TType* PType;
struct TType;
typedef	const struct TType* PType;

struct TObject;
typedef struct TObject* PObject;

struct TEvent;
typedef	struct TEvent* PEvent;

struct TVar;
typedef	struct TVar* PVar;

typedef	void	(*TSetVarProc)(PObject obj, PVar var);
typedef	void	(*TMainProc)(PObject obj);
typedef	void	(*TEventCmdProc)(PEvent ev, char* param);
typedef	void	(*TVarCmdProc)(PObject obj, PVar var, char* param);
typedef	void	(*TObjectCmdProc)(PObject obj, char* param);

struct tagVarCmdDef
{
	char*		Name;
	TVarCmdProc	Proc;
};
typedef struct tagVarCmdDef TVarCmdDef;
typedef TVarCmdDef* PVarCmdDef;


struct TVarDef
{
	char*			Name;
	TSetVarProc		Proc;
	PVarCmdDef		VarCmds;

	u8				ValueSize;
	char*			InitialValue;
};
typedef	const struct TVarDef* PVarDef;

struct TEventDef
{
	char*			Name;
	u8				dummy;
};
typedef const struct TEventDef* PEventDef;

struct tagObjectCmdDef
{
	char*			Name;
	TObjectCmdProc	Proc;
};
typedef struct tagObjectCmdDef	TObjectCmdDef;
typedef TObjectCmdDef* 			PObjectCmdDef;

struct TType
{
	char*			Name;

	TMainProc		MainProc;

	// процедура инициализации, вызывается только один раз,
	// сразу после конструирования объекта
	TMainProc		InitProc;

	PObjectCmdDef	ObjCmds;		// команды для объектов

	u8				VarCount;
	PVarDef     	VarDefs;
	
	u8				EventCount;
	PEventDef   	EventDefs;
	
	u16				UserDataSize;
};

// Object /////////////////////////////////////////////////////////////

struct TVar
{
	PVarDef		VarDef;
	char*		Value;
//	char*		CPUPinName;
	int			CPUPinNumber;
};

typedef struct tagMsgTemplate TMsgTemplate;
typedef TMsgTemplate* PMsgTemplate;

struct tagMsgTemplate
{
//	u32			ip;
	char*		Addresss;
	char*		Param;
	PMsgTemplate next;
};

struct TEvent
{
	PEventDef		EventDef;
	PMsgTemplate	Msgs;
	
	u32				PsTime;
};

struct TObject
{
	char*		Name;
	PType		Type;
	PVar*		Vars;
	PEvent*		Events;
	
	void* 		UserData;
	
	bool		InputsChanged;
	
	PObject ObjList;	// список дочерних объектов
	
	PObject next; 		// указатель на следующий объект в списке объектов родителя

};

// команды для событий и переменных
struct tagEventCmdDef
{
	char*			Name;
	TEventCmdProc	Proc;
};
typedef struct tagEventCmdDef TEventCmdDef;

#define CMD_EVENT_COUNT	3
//#define CMD_VAR_COUNT	3

extern const TEventCmdDef EventCmdDef[CMD_EVENT_COUNT];
//extern const PVarCmdDef 	VarCmdDef;//[CMD_VAR_COUNT];
extern const TVarCmdDef DefaultVarCmdDef[];
extern const TObjectCmdDef DefaultObjectCmdDef[];

#endif

