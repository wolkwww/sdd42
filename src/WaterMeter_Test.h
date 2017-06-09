/*
 * WaterMeter_Test_Loop.h
 *
 *  Created on: 12 апр. 2016 г.
 *      Author: mrhru
 */

#ifndef SRC_WATERMETER_TEST_H_
#define SRC_WATERMETER_TEST_H_

#define SPEC_FREQ_MIN		70
#define SPEC_FREQ_MAX		75


// кол-во счетчиков в одном канале
#define	METERS_PER_CHANNEL 	8
// количество независимых параллельных каналов работы
#define CHANNEL_COUNT			3


#define SENDER_NAME_SIZE 	9
#define SENDER_ADDR_SIZE	5	// fixed
#define MAX_LENGTH_ID		9

typedef struct
{
		i64 Ticks;
		double Value;
} TPointInfo;

typedef struct
{
		TPointInfo Before;
		TPointInfo After;
} TEventInfo;

typedef enum
{
	comstNone,
	comstWaitForSend,
	comstSendOK,
	comstSendError
} CommandState;

typedef struct
{
		u8 Name;
		u64 Param;
		CommandState State;
} TSenderCommand;

typedef struct
{
		char Name[SENDER_NAME_SIZE + 1];
		u8 Addr[SENDER_ADDR_SIZE];
		char sID[MAX_LENGTH_ID + 1];

		int ch; //

		TEventInfo Start;
		TEventInfo Stop;
		TEventInfo ForFreq;

		bool Stopped;

		double Result;
		double F;

		u64 totalDelta;
		u64 totalDeltaCount;
		i64 minTicksDelta;
		i64 maxTicksDelta;
		TSenderCommand Cmd;
} TSenderData;

typedef TSenderData* PSenderData;

typedef struct
{
		TSenderData Senders[METERS_PER_CHANNEL];
		i64 StartTicks;
		i64 StopTicks;

		bool BeginMeasure;
		bool DoneMeasure;
} TMetersData;

extern TMetersData MetersData[CHANNEL_COUNT];

void WaterMeter_Init(void);

void WaterMeter_InitMeasure(int ch, i64 startTicks, i64 duration);
void WaterMeter_Step(void);
bool WaterMeter_IsAllSendersStopped(int ch);
void WaterMeter_StopMeasure(int ch);

void WaterMeter_ForgetSenders(int ch);

void WaterMeter_SetCmd(int ch, int nMeter, u8 Cmd, char* sParams);

void WaterMeter_CalcResults(void);
void WaterMeter_CalcF(void);
void WaterMeter_ShowResults(void);
int WaterMeter_GetTimeToLeftInSeconds(int ch);

void WaterMeter_Test_Step(void);
void WaterMeter_Test_Loop(void);

#endif /* SRC_WATERMETER_TEST_H_ */
