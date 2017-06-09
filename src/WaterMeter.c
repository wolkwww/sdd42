/*
 * WaterMeter.c
 *
 *  Created on: 11 ���. 2016 �.
 *      Author: mrhru
 */

#include "defines.h"
#ifdef WATER_STATION

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "common.h"
#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "HAL.h"

#include "WaterMeter.h"
#include "WaterMeter_Test.h"

#define	N_CHANNEL        	"CHANNEL"
#define	N_START          	"START"
#define	N_STOP           	"STOP"
#define	N_GETF				"GETF"
#define	N_FORGET			"FORGET"

#define	N_DONE           	"DONE"
#define	N_TIMETOLEFT		"TIMETOLEFT"

#define	N_INTERVAL       	"INTERVAL"
#define	N_DELAY       		"DELAY"

#define	N_TIMEOUT        	"TIMEOUT"

const char* const N_R[METERS_PER_CHANNEL] =
{"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7"};

const char* N_F[METERS_PER_CHANNEL] =
{"F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7"};

#define CMD_NAME_TEMPLATE	"CMD_"

const char* N_CMD[METERS_PER_CHANNEL] =
{"CMD0", "CMD1", "CMD2", "CMD3", "CMD4", "CMD5", "CMD6", "CMD7", };

#define	N_ON_START       	"ON_START"
#define	N_ON_STOP       	"ON_STOP"
#define	N_ON_DONE        	"ON_DONE"

#define	N_WATERMETER      	"WATERMETER"

/*
 * �������� ������.
 * ��������� �� ������� ������� ����� ���������� ������� ��������� ���� ���������,
 * ������ ����������� ������� �� ������������ ������ ����, ����� �������� �����
 * ������ ����������� ������� �� �������� ������������ ������ ���� � ��������� ��������
 * ��������� ���� ���������.
 *
 * ������ ������� �������� �� ����� ���� ������� �������� � ����:
 * _XXXXXXXX=YYYYYYYYY
 * ���	_XXXXXXXX	-	UID ��������
 * 		YYYYYYYYY	-	����������� ��������, �������� ��� �����
 * ��� �������� ����������� nRF24.
 *
 * ��� ��� ������� �� ������ ��������� ���������� �� ��������� � �������� ������ ��������� ���������,
 * � ���-�� �������� ������ ������� ��� �������� ��-�� �� �����������, ���������� ����������� ������������
 * ��������.
 * �������� ������ ����������� � dictionary<string UID, <EventStruc Start, Stop>>
 * ���
 * 	EventStruc = {PointStruct Before, After}
 * 	PointStruct = {Ticks, Value}
 * 		���
 * 		UID	-	_XXXXXXXX
 *
 * PointStruct{Value, Ticks} - ������ ������ �������� �� �������� � ���� ��������
 * EventStruc{PointStruct Before, After} - ������� � �������� ��������������� �� � ����� ��������� �������,
 * 		��� ���������� ��� ������������ �������� ������������ ������� ����������� �������
 *
 * ������� ����������� ������� Start � Stop ����������� �������� ��� StartTicks � StopTicks
 *
 * �� ������� ������-���� �������, Start ��� Stop, �������� �������� �� ��������� ����������� � ���������������
 * ����� Start.Before ��� Stop.Before.
 */

typedef enum
{
	wsStopped,
	wsBeforeStartTime,
	wsBeforeStopTime,
	wsWaitForDone,
} WaterStates;

struct TWaterMeterUserData
{
	int OldState;
	u64 startTicks;
	u64 stopTicks;

	int ch;
	u32 timeout;

	WaterStates state;
};

typedef struct TWaterMeterUserData* PWaterMeterUserData;

#define udata 			((PWaterMeterUserData) obj->UserData)

static void Proc_INIT(PObject obj)
{
	udata->ch = -1;
}

static void Proc_FORGET(PObject obj, PVar var)
{
	if (udata->ch >= 0)
	{
		WaterMeter_ForgetSenders(udata->ch);
	}
}

static void ResetResults(PObject obj)
{
	for (int i = 0; i < METERS_PER_CHANNEL; i ++)
	{
		SetVarValueAsInt(obj, N_R[i], 0);
	}
}

static void SetResults(PObject obj)
{
	if (udata->ch >= 0)
	{
		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			SetVarValueAsDouble(obj, N_R[i], MetersData[udata->ch].Senders[i].Result);
		}
	}
}

static void Proc_START(PObject obj, PVar var)
{
	if (udata->ch >= 0)
	{
		udata->timeout = ValueAsInt(GetObjVarValue(obj, N_TIMEOUT)) * 10;

		i32 interval = ValueAsInt(GetObjVarValue(obj, N_INTERVAL));

		udata->OldState = false;

		u64 duration = interval * 10;

		i32 delayBefore = ValueAsInt(GetObjVarValue(obj, N_DELAY)) * 10;

		// ����� ���������� �� ����� ������
		//	pud->startTicks = ((GetTicks() + delayBefore) / 10000) * 10000;

		udata->startTicks = GetTicks() + delayBefore;
		udata->stopTicks = udata->startTicks + duration;

		WaterMeter_InitMeasure(udata->ch, udata->startTicks, duration);

		udata->state = wsBeforeStartTime;
		SetVarValueAsInt(obj, N_DONE, 0);
		SetVarValueAsInt(obj, N_START, 1);

		ResetResults(obj);

		msgn("Measure Started");
	}
}

static void Proc_STOP(PObject obj, PVar var)
{
	if (udata->ch >= 0)
	{
		udata->state = wsStopped;

		WaterMeter_StopMeasure(udata->ch);

		SetVarValueAsInt(obj, N_START, 0);
		SetVarValueAsInt(obj, N_DONE, 1);

		msgn("Measure Stopped");
	}
}

// ������ �������
// CMD:PARAMS
static void Proc_CMD(PObject obj, PVar var)
{
	if (udata->ch >= 0)
	{
		char* sCmd = GetVarValue(var);

		char* sParams = strchr((char*) sCmd, ':');
		if (sParams != NULL)
		{
			// svalue ��������� �����
			*sParams = '\0';
			sParams ++;
		}

		u8 Cmd = sCmd[0];

		// ����� �������� �� ����� ����������
		int nMeter = var->VarDef->Name[strlen(CMD_NAME_TEMPLATE) - 1] - '0';

		printf("CMD:\t|%s| \tmeter:\%d \tCMD:%c \tPARAMS:|%s|\r\n", sCmd, nMeter, Cmd, sParams);

		WaterMeter_SetCmd(udata->ch, nMeter, Cmd, sParams);

		SetVarAsInt(var, MetersData[udata->ch].Senders[nMeter].Cmd.State);
	}
}

static void Proc_MAIN(PObject obj)
{
	if (udata->ch >= 0)
	{
		u64 currticks;
		bool newState;

		WaterMeter_Step();

		switch (udata->state)
		{
			case wsStopped:
				break;

			case wsBeforeStartTime:
				currticks = GetTicks();
				if (currticks > udata->startTicks)
				{
					// ������ ������������ ������
					FireEvent(obj, N_ON_START);

					udata->state = wsBeforeStopTime;
				}
				break;

			case wsBeforeStopTime:
				currticks = GetTicks();
				if (currticks > udata->stopTicks)
				{
					// ������ ��������� ������������ ������
					FireEvent(obj, N_ON_STOP);

					udata->state = wsWaitForDone;
				}
				break;

			case wsWaitForDone:
				newState = WaterMeter_IsAllSendersStopped(udata->ch);

				if (newState && ( !udata->OldState))
				{
					udata->OldState = true;

					SetVarValueAsInt(obj, N_DONE, 1);
					SetVarValueAsInt(obj, N_START, 0);

					FireEvent(obj, N_ON_DONE);
					msgn("WM: Measure done");
					WaterMeter_ShowResults();

					SetResults(obj);

					udata->state = wsStopped;
					WaterMeter_StopMeasure(udata->ch);
				}
				else
				{
					// �������� �� �������
					u64 currticks = GetTicks();
					if (currticks > (udata->stopTicks + udata->timeout))
					{
						SetVarValueAsInt(obj, N_DONE, 1);
						SetVarValueAsInt(obj, N_START, 0);

						FireEvent(obj, N_ON_DONE);
						msgn("WM: Timeout");

						WaterMeter_ShowResults();

						SetResults(obj);

						udata->state = wsStopped;
						WaterMeter_StopMeasure(udata->ch);
					}
				}
				break;
		}

		SetVarValueAsInt(obj, N_TIMETOLEFT, WaterMeter_GetTimeToLeftInSeconds(udata->ch));

		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			SetVarValueAsInt(obj, N_CMD[i], MetersData[udata->ch].Senders[i].Cmd.State);
		}
	}
}

static void Proc_GETF(PObject obj, PVar var)
{
	if (udata->ch >= 0)
	{
		WaterMeter_CalcF();

		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			SetVarValueAsDouble(obj, N_F[i], MetersData[udata->ch].Senders[i].F);
		}
	}
}

static void Proc_CHANNEL(PObject obj, PVar var)
{
	udata->ch = ValueAsInt(GetObjVarValue(obj, N_CHANNEL));
}

const struct TVarDef WaterMeterVarDef[] =
{
{N_CHANNEL, Proc_CHANNEL, null, 3, "-1"},			// ����� 0..2 (?) - ��

    {N_FORGET, Proc_FORGET, null, 0, ""},		// ������ ��� ��������

    {N_START, Proc_START, null, 3, "0"},		// ����� ���������
    {N_STOP, Proc_STOP, null, 0, ""},			// �������������� �������

    {N_GETF, Proc_GETF, null, 0, ""},			// ������ �� ���������� �������

    {N_DONE, null, null, 2, "0"},				// ������� ���������� ���������
    {N_TIMETOLEFT, null, null, 8, "0"},			// ����� �� ����� ��������� � ���.

    {N_INTERVAL, null, null, 8, "3000"},		// ������������ ���������
    {N_DELAY, null, null, 8, "2000"},			// �������� ����� ����������

    {N_TIMEOUT, null, null, 8, "1000"},			// ����� �������� ����� ��������� ����� ����������

    {"R0", null, null, 12, "0"},				// ��������� ��������� 0
    {"R1", null, null, 12, "0"},				// ��������� ��������� 1
    {"R2", null, null, 12, "0"},				// ��������� ��������� 2
    {"R3", null, null, 12, "0"},				// ��������� ��������� 3
    {"R4", null, null, 12, "0"},				// ��������� ��������� 4
    {"R5", null, null, 12, "0"},				// ��������� ��������� 5
    {"R6", null, null, 12, "0"},				// ��������� ��������� 6
    {"R7", null, null, 12, "0"},				// ��������� ��������� 7

    {"F0", null, null, 12, "0"},				// ������� 0
    {"F1", null, null, 12, "0"},				// ������� 1
    {"F2", null, null, 12, "0"},				// ������� 2
    {"F3", null, null, 12, "0"},				// ������� 3
    {"F4", null, null, 12, "0"},				// ������� 4
    {"F5", null, null, 12, "0"},				// ������� 5
    {"F6", null, null, 12, "0"},				// ������� 6
    {"F7", null, null, 12, "0"},				// ������� 7

    {"CMD0", Proc_CMD, null, 20, "0"},		// ������� � ����������� 0
    {"CMD1", Proc_CMD, null, 20, "0"},		// ������� � ����������� 1
    {"CMD2", Proc_CMD, null, 20, "0"},		// ������� � ����������� 2
    {"CMD3", Proc_CMD, null, 20, "0"},		// ������� � ����������� 3
    {"CMD4", Proc_CMD, null, 20, "0"},		// ������� � ����������� 4
    {"CMD5", Proc_CMD, null, 20, "0"},		// ������� � ����������� 5
    {"CMD6", Proc_CMD, null, 20, "0"},		// ������� � ����������� 6
    {"CMD7", Proc_CMD, null, 20, "0"},		// ������� � ����������� 7

    };

const struct TEventDef WaterMeterEventDef[] =
{
{N_ON_START, 0},	//
    {N_ON_STOP, 0},	//
    {N_ON_DONE, 0},	//
    };

const struct TType TWaterMeter =
{N_WATERMETER, Proc_MAIN,	// Proc_MAIN,
    Proc_INIT,	// InitProc

    null,

    (sizeof(WaterMeterVarDef) / sizeof(struct TVarDef)),
    WaterMeterVarDef,

    (sizeof(WaterMeterEventDef) / sizeof(struct TEventDef)),
    WaterMeterEventDef,

    sizeof(struct TWaterMeterUserData),	//
    };

#endif
