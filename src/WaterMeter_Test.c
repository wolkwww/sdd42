/*
 * WaterMeter_Test_Loop.c
 *
 *  Created on: 12 апр. 2016 г.
 *      Author: mrhru
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "lpc_types.h"

//#include "usart.h"
#include "HAL.h"
#include "common.h"
#include "ntp.h"
#include "tm_stm32f3_nrf24l01.h"

#include "WaterMeter_Test.h"

bool SendCmd(PSenderData sender);
void ProcessCmd(PSenderData sender);

#define RF_CHANNEL_CHECKING				112
#define CHECKING_ADDRESS 				"CHECK"

/* Принятые пакеты сохраняются в dictionary<string UID, <TEventInfo Start, Stop>>
 * где
 * 	TEventInfo = {TPointInfo Before, After}
 * 	TPointInfo = {Value, Ticks}
 * 		где
 * 		UID	-	_XXXXXXXX
 *
 * TPointInfo{Value, Ticks} - момент приема значения от счетчика и само значение
 * TEventInfo{TPointInfo Before, After} - времена и значения непосредственно до и после заданного события,
 */

TMetersData MetersData[CHANNEL_COUNT];

#define RB_BUS_SIZE	33

typedef enum
{
	BeforeStart,
	Start,
	BeforeStop,
	Stop
} StateState;

#define BEFORE_START_TIME		10000
#define BEFORE_STOP_TIME		10000

void InitSenders(int ch);
void ProcessPacket(u8* pack, i64 ticks);
void ShowSendersInfo(void);
void WaterMeter_ShowResults(void);
void TestStates(u8* rb);

bool TestHex(const char* s)
{
	// "_0..9A..F"
	while ( *s)
	{
		u8 c = *s;
		if ((c < '0') || ((c > '9') && (c < 'A')) || ((c > 'F') && (c != '_')))
		{
			return FALSE;
		}
		s ++;
	}
	return TRUE;
}

bool TestInt(const char* s)
{
	// "0..9"
	while ( *s)
	{
		u8 c = *s;
		if ((c < '0') || (c > '9'))
		{
			return FALSE;
		}
		s ++;
	}
	return TRUE;
}

static void Init_nRF(void)
{
	TM_NRF24L01_Init(RF_CHANNEL_CHECKING, 32, 15);

	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_2M, TM_NRF24L01_OutputPower_M18dBm);
//	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_250k, TM_NRF24L01_OutputPower_0dBm);

	TM_NRF24L01_SetMyAddress((u8*) CHECKING_ADDRESS);

	TM_NRF24L01_SetTxAddress((u8*) CHECKING_ADDRESS);

	ShowRegs();
	delay(1000); //TODO TEST REAL 1000
}

bool WaterMeter_IsAllSendersStopped(int ch)
{
	for (int i = 0; i < METERS_PER_CHANNEL; i ++)
	{
		if (MetersData[ch].Senders[i].Name[0] != '\0')
		{
			if ( !MetersData[ch].Senders[i].Stopped)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

void WaterMeter_StopMeasure(int ch)
{
	MetersData[ch].BeginMeasure = FALSE;
}

void WaterMeter_CalcResults(void)
{
	for (int ch = 0; ch < CHANNEL_COUNT; ch ++)
	{
		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			if (MetersData[ch].Senders[i].Name[0] != '\0')
			{
				PSenderData s = &MetersData[ch].Senders[i];

				double startV = s->Start.Before.Value
				    + (((double) (MetersData[ch].StartTicks - s->Start.Before.Ticks)) / ((double) (s->Start.After.Ticks - s->Start.Before.Ticks))) * (s->Start.After.Value
				        - s->Start.Before.Value);

				double stopV = s->Stop.Before.Value
				    + (((double) (MetersData[ch].StopTicks - s->Stop.Before.Ticks)) / ((double) (s->Stop.After.Ticks - s->Stop.Before.Ticks))) * (s->Stop.After.Value
				        - s->Stop.Before.Value);

				s->Result = stopV - startV;

//			printf("startV:\t%f\tstopV:\t%f\tResult:\t%f\r\n", startV, stopV, s->Result);
			}
		}
	}
}

//11930692637819553
//1677721600000000

void WaterMeter_CalcF(void)
{
	i64 currticks = (i64) GetTicks();

	for (int ch = 0; ch < CHANNEL_COUNT; ch ++)
	{
		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			PSenderData s = &MetersData[ch].Senders[i];
			s->F = -1.0;

			if (MetersData[ch].Senders[i].Name[0] != '\0')
			{
				if ((s->ForFreq.After.Ticks != 0) && (s->ForFreq.Before.Ticks != 0) && (s->ForFreq.After.Ticks != s->ForFreq.Before.Ticks))
				{
					i64 deltaTicks = s->ForFreq.After.Ticks - s->ForFreq.Before.Ticks;

					if ((currticks - s->ForFreq.After.Ticks) < (deltaTicks * 5))
					{
						s->F = (s->ForFreq.After.Value - s->ForFreq.Before.Value) / (double) (deltaTicks) * 10000.0;
					}
					else
					{
						s->F = 0.0;
					}
				}
			}
		}
	}
}

//static void TestSpecialFrequency(void)
//{
//	for (int i = 0; i < SENDERS_MAX_COUNT; i ++)
//	{
//		PSenderData s = &MetersData[ch].Senders[i];
//
//		if (s->Name[0] != '\0')
//		{
//			if ((s->F >= SPEC_FREQ_MIN) && (s->F <= SPEC_FREQ_MAX))
//			{
//				BeepEnable = TRUE;
//				return;
//			}
//		}
//	}
//
//	BeepEnable = FALSE;
//}
//

void WaterMeter_ShowResults(void)
{
	WaterMeter_CalcResults();

	for (int ch = 0; ch < CHANNEL_COUNT; ch ++)
	{
		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			if (MetersData[ch].Senders[i].Name[0] != '\0')
			{
				PSenderData s = &MetersData[ch].Senders[i];

				printf("%f\t", s->Result);
			}
		}
		CR();

		printf("Times:\t%llu \t%llu \t%llu", MetersData[ch].StartTicks, MetersData[ch].StopTicks, MetersData[ch].StopTicks - MetersData[ch].StartTicks);
		CR();

		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			if (MetersData[ch].Senders[i].Name[0] != '\0')
			{
				PSenderData s = &MetersData[ch].Senders[i];

				printf("%d) %s:\t", i, s->Name);
				printf("%llu\t%llu\t%f\t%f\t", //
				    s->Start.Before.Ticks, s->Start.After.Ticks, //
				    s->Start.Before.Value, s->Start.After.Value);
				printf("%llu\t%llu\t%f\t%f \t", //
				    s->Stop.Before.Ticks, s->Stop.After.Ticks, //
				    s->Stop.Before.Value, s->Stop.After.Value);
				printf("%f\tmax:%llu\tmin:%llu\r\n", //
				    s->Result, //
				    s->maxTicksDelta, s->minTicksDelta);

			}
		}
	}
	CR();
}

//i	Name	Start.Before.Ticks	Start.After.Ticks	Start.Before.Value	Start.After.Value	Stop.Before.Ticks	Stop.After.Ticks	Stop.Before.Value	Stop.After.Value	Result

void ShowSendersInfo(void)
{
	for (int ch = 0; ch < CHANNEL_COUNT; ch ++)
	{
		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			if (MetersData[ch].Senders[i].Name[0] != '\0')
			{
//			printf("%d)\t%s: val = %u\tmin = %u\tmax = %u\tave = %u\r\n", i, MetersData[ch].Senders[i].Name, MetersData[ch].Senders[i].Start.Before.Value,
//			       MetersData[ch].Senders[i].minTicksDelta, MetersData[ch].Senders[i].maxTicksDelta, MetersData[ch].Senders[i].totalDelta / MetersData[ch].Senders[i].totalDeltaCount);
				printf("%d)\t%s: val = %f | %f (%f)\r\n", i, MetersData[ch].Senders[i].Name, MetersData[ch].Senders[i].Start.Before.Value,
				       MetersData[ch].Senders[i].Start.After.Value, MetersData[ch].Senders[i].Start.After.Value - MetersData[ch].Senders[i].Start.Before.Value);
			}
		}
	}
	CR();
	CR();
}

void InitSenders(int ch)
{
	for (int i = 0; i < METERS_PER_CHANNEL; i ++)
	{
		MetersData[ch].Senders[i].Name[0] = '\0';

		MetersData[ch].Senders[i].Cmd.Name = 0;
		MetersData[ch].Senders[i].Cmd.Param = 0;

		MetersData[ch].Senders[i].Start.Before.Ticks = 0;
		MetersData[ch].Senders[i].Start.Before.Value = 0.0;
		MetersData[ch].Senders[i].Start.After.Ticks = 0;
		MetersData[ch].Senders[i].Start.After.Value = 0.0;

		MetersData[ch].Senders[i].Stop.Before.Ticks = 0;
		MetersData[ch].Senders[i].Stop.Before.Value = 0.0;
		MetersData[ch].Senders[i].Stop.After.Ticks = 0;
		MetersData[ch].Senders[i].Stop.After.Value = 0.0;

		MetersData[ch].Senders[i].minTicksDelta = 1000000;
		MetersData[ch].Senders[i].maxTicksDelta = 0;

		MetersData[ch].Senders[i].totalDelta = 0;
		MetersData[ch].Senders[i].totalDeltaCount = 0;
	}

	MetersData[ch].BeginMeasure = FALSE;
	MetersData[ch].DoneMeasure = FALSE;
	MetersData[ch].StartTicks = 0;
	MetersData[ch].StopTicks = 0;

}

void MakeAddrFromName(PSenderData sender)
{
	u8 HexToU8(u8 hex)
	{
		if (hex <= '9')
		{
			return (hex - '0');
		}
		else
		{
			return (hex - 'A' + 10);
		}
	}

	u8 HexHexToU8(u8 hexHigh, u8 hexLow)
	{
		return (HexToU8(hexHigh) << 4) | HexToU8(hexLow);
	}

	if (sender->Name[0] != '\0')
	{
		sender->Addr[0] = '_';

		for (int i = 1; i < SENDER_ADDR_SIZE; i ++)
		{
			// i	i*2-1
			// 1	1
			// 2	3
			// 3	5
			sender->Addr[i] = HexHexToU8(sender->Name[i * 2 - 1], sender->Name[i * 2]);
		}
	}

}

PSenderData FindSender(char* name)
{
	for (int ch = 0; ch < CHANNEL_COUNT; ch ++)
	{
		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			if (MetersData[ch].Senders[i].Name[0] != '\0')
			{
				if (strcmp(MetersData[ch].Senders[i].Name, name) == 0)
				{
					return &MetersData[ch].Senders[i];
				}
			}
		}
	}

	// not found
	// make new entry
	for (int ch = 0; ch < CHANNEL_COUNT; ch ++)
	{
		for (int i = 0; i < METERS_PER_CHANNEL; i ++)
		{
			if (MetersData[ch].Senders[i].Name[0] == '\0')
			{
				printf("%d) ADD: %s", i, MetersData[ch].Senders[i].Name);

				strncpy(MetersData[ch].Senders[i].Name, name, SENDER_NAME_SIZE);

				MakeAddrFromName( &MetersData[ch].Senders[i]);

				MetersData[ch].Senders[i].ch = ch;

				return &MetersData[ch].Senders[i];
			}
		}
	}
	return NULL;
}

void WaterMeter_ForgetSenders(int ch)
{
	InitSenders(ch);
}

void WaterMeter_InitMeasure(int ch, i64 startTicks, i64 duration)
{
	MetersData[ch].StartTicks = startTicks;
	MetersData[ch].StopTicks = MetersData[ch].StartTicks + duration;
	MetersData[ch].BeginMeasure = TRUE;

	for (int i = 0; i < METERS_PER_CHANNEL; i ++)
	{
		if (MetersData[ch].Senders[i].Name[0] != '\0')
		{
			// нежелательно обнулять! при низкой скорости передачи,
			// до StartTicks может не оказаться данных
			// Start.Before.Ticks и Start.Before.Value!
			// !! нормально не работает! надо обнулять!
			// но TIMEOUT, задержку перед началом измерений, надо
			// делать больше, чем период сигнала, иначе измерение
			// может начаться без значений в Start.Before
			// не будет работать нормально вариант ожидания самого первого
			// сигнала, но так как счетчиков много, то надо ожидать первых
			// сигналов от всех, но не все могут работать, поэтому может
			// произойти "зависание". Что потребует введение еще и таймаута,
			// но он и так есть...
			// Например, при частоте сигнала 0.35 Гц (наименьшая зарегистрированная)
			// TIMEOUT должен быть > 1/0.35 = 2.85 сек, т.е. > 3 сек
			// время измерения
			MetersData[ch].Senders[i].Start.Before.Ticks = 0;
			MetersData[ch].Senders[i].Start.Before.Value = 0.0;
			MetersData[ch].Senders[i].Start.After.Ticks = 0;
			MetersData[ch].Senders[i].Start.After.Value = 0.0;

			MetersData[ch].Senders[i].Stop.Before.Ticks = 0;
			MetersData[ch].Senders[i].Stop.Before.Value = 0.0;
			MetersData[ch].Senders[i].Stop.After.Ticks = 0;
			MetersData[ch].Senders[i].Stop.After.Value = 0.0;

			MetersData[ch].Senders[i].minTicksDelta = 1000000;
			MetersData[ch].Senders[i].maxTicksDelta = 0;

			MetersData[ch].Senders[i].Stopped = FALSE;
		}
	}

//	printf("START NEW MEASURE\r\n%u -> %u\r\n", MetersData[ch].StartTicks, MetersData[ch].StopTicks);
}

//			start			stop
//			  |				  |
//	0	1	2	3	4	5	6	7	8	9
//	*	0	1	2
//	0	1	2	3
//	*	*	*	*	*	4	5	6
//	*	*	*	*	4	5	6	7
void SetNewSenderData(PSenderData sender, const char* sID, i64 ticks, double value)
{
//	printf("%s\t%llu \t%llu\r\n", sender->Name, ticks, value);

	strncpy(sender->sID, sID, MAX_LENGTH_ID);

	// для измерения частоты
	sender->ForFreq.Before.Ticks = sender->ForFreq.After.Ticks;
	sender->ForFreq.Before.Value = sender->ForFreq.After.Value;
	sender->ForFreq.After.Ticks = ticks;
	sender->ForFreq.After.Value = value;

	sender->F = -1.0;

	if ((sender->ForFreq.After.Ticks != 0) && (sender->ForFreq.Before.Ticks != 0) && (sender->ForFreq.After.Ticks != sender->ForFreq.Before.Ticks))
	{
		i64 deltaTicks = sender->ForFreq.After.Ticks - sender->ForFreq.Before.Ticks;

		if (deltaTicks < sender->minTicksDelta)
		{

			sender->minTicksDelta = deltaTicks;
		}
		else
		{
			if (deltaTicks > sender->maxTicksDelta)
			{
				sender->maxTicksDelta = deltaTicks;
			}
		}

//		printf("%llu\t%llu\t%llu\t\r\n", deltaTicks, sender->minTicksDelta, sender->maxTicksDelta);

		i64 timeout = ticks - sender->ForFreq.After.Ticks;
		if ((timeout < 33000) || (timeout < (deltaTicks * 30)))
		{
			sender->F = (sender->ForFreq.After.Value - sender->ForFreq.Before.Value) / (double) (deltaTicks) * 10000.0;
		}
		else
		{
			sender->F = 0.0;
		}
	}

	int ch = sender->ch;

	if ((MetersData[ch].BeginMeasure != FALSE) && (sender->Stopped == FALSE))
	{
		if (ticks < MetersData[ch].StartTicks)	// до
		{
			// постоянное запоминание в Start последних двух значений
			sender->Start.Before.Ticks = ticks;
			sender->Start.Before.Value = value;
			sender->Start.After.Ticks = 0;
			sender->Start.After.Value = 0.0;
		}
		else
		{
			if ((ticks >= MetersData[ch].StartTicks) && (ticks < MetersData[ch].StopTicks)) // между
			{
				if (sender->Start.After.Ticks == 0) // первое срабатывание
				{
					sender->Start.After.Ticks = ticks;
					sender->Start.After.Value = value;
				}
				// и копирование в Stop, вдруг больше не будет
				sender->Stop.Before.Ticks = ticks;
				sender->Stop.Before.Value = value;
			}
			else // после
			{
				sender->Stop.After.Ticks = ticks;
				sender->Stop.After.Value = value;

				sender->Stopped = TRUE;
			}
		}
	}
	else
	{
//		sender->Start.Before.Ticks = sender->Start.After.Ticks;
//		sender->Start.Before.Value = sender->Start.After.Value;
//		sender->Start.After.Ticks = ticks;
//		sender->Start.After.Value = value;
	}
}

int WaterMeter_GetTimeToLeftInSeconds(int ch)
{
	i64 ticks = (i64) GetTicks();

	if (MetersData[ch].StopTicks > ticks)
	{
		return (MetersData[ch].StopTicks - ticks) / 10000;
	}
	else
	{
		return 0;
	}
}

void ProcessPacket(u8* pack, i64 ticks)
{
//	msgn((char*) pack);

	// формат пакета
	// name:value;serialID

	char* name = (char*) pack;

	// поиск конца имени
	char* svalue = strchr((char*) pack, ':');
	if (svalue != NULL)
	{
		// name завершаем нулем
		*svalue = '\0';

		if (strlen(name) == SENDER_NAME_SIZE)
		{
			// указывает на value
			svalue ++;

//			printf("%s = %s\r\n", name, svalue);

			// поиск конца value
			char* sID = strchr((char*) svalue, ';');
			if (sID != NULL)
			{
				// svalue завершаем нулем
				*sID = '\0';
				sID ++;
			}

			if (TestHex(name) && TestHex(svalue))
			{
//				double value = atoll(svalue) / 256.0 / 256.0 / 256.0;
				double value = HexToU64(svalue) / 256.0 / 256.0 / 256.0;

				PSenderData sender = FindSender(name);
				if (sender != NULL)
				{
//					msg(sender->Name);
//					msg(" [");
//					printf("%c", sender->Addr[0]);
//					prn8hex(sender->Addr[1]);
//					prn8hex(sender->Addr[2]);
//					prn8hex(sender->Addr[3]);
//					prn8hex(sender->Addr[4]);
//					msg("] ");
//					printf("(%s)\t%llu \t%llu\r\n", sID, ticks, value);

					SetNewSenderData(sender, sID, ticks, value);

					ProcessCmd(sender);

				}
				else
				{
//					LED_TOGGLE(LRed1);
				}
			}
		}
	}
}

static u8 rb[RB_BUS_SIZE];

void WaterMeter_Init(void)
{
	Init_nRF();

	for (int ch = 0; ch < CHANNEL_COUNT; ch ++)
	{
		InitSenders(ch);
	}
	WaterMeter_InitMeasure(0, ((GetTicks() + 10000L) / 10000L) * 10000L, 10000L);
}

void WaterMeter_Test_Step(void)
{
	if (TM_NRF24L01_DataReady())
	{
		i64 ticks = GetTicks();

		LED1_ON();
		do
		{

			TM_NRF24L01_GetData(rb);

			msgn(rb);

			ProcessPacket(rb, ticks);
		}
		while (TM_NRF24L01_DataReady());
		LED1_OFF();

		TM_NRF24L01_PowerUpRx();
	}

	if (WaterMeter_IsAllSendersStopped(0))
	{
//		WaterMeter_ShowResults();
//		WaterMeter_InitMeasure(0, ((GetTicks() + 10000L) / 10000L) * 10000L, 10000L);
	}
}

void WaterMeter_Test_Loop(void)
{
	WaterMeter_Init();

	while (1)
	{
		WaterMeter_Test_Step();

//		ntp_process();

		WDTReset();
	}
}

void WaterMeter_SetCmd(int ch, int nMeter, u8 Cmd, char* sParams)
{
	if (nMeter < METERS_PER_CHANNEL)
	{
		PSenderData sender = &MetersData[ch].Senders[nMeter];

		sender->Cmd.Name = Cmd;
		sender->Cmd.Param = atoll(sParams);
		sender->Cmd.State = comstWaitForSend;
	}
}

void ProcessCmd(PSenderData sender)
{
	int i;

//	TEST1_OFF();
//	TEST2_ON();
	if (sender->Cmd.Name != 0)
	{
//		TEST1_ON();
		for (i = 0; i < 8; i ++)
		{
			if (SendCmd(sender))
			{
				sender->Cmd.Name = 0;
				sender->Cmd.Param = 0;
				break;
			}
		}
//		TEST1_OFF();
	}
//	TEST2_OFF();
//	TEST1_ON();
}

bool SendCmd(PSenderData sender)
{
//	TEST1_ON();
	//Set TX address, 5 bytes
	TM_NRF24L01_SetTxAddress(sender->Addr);

	TM_NRF24L01_PowerUpTx();
	//Transmit data, goes automatically to TX mode
	char buf[33];
	sprintf(buf, "CMD:%c:%llu", sender->Cmd.Name, sender->Cmd.Param);

	TM_NRF24L01_Transmit((u8*) buf);

//	msgn("START TRANSMIT");
	//Wait for data to be sent
	TM_NRF24L01_Transmit_Status_t transmissionStatus;
	do
	{
		transmissionStatus = TM_NRF24L01_GetTransmissionStatus();
	}
	while (transmissionStatus == TM_NRF24L01_Transmit_Status_Sending);
//	TEST1_OFF();

	printf("send CMD: |%s|\t", buf);
//	msgn("END TRANSMIT");

	if (transmissionStatus == TM_NRF24L01_Transmit_Status_Ok)
	{
		msgn("TRANSMIT OK");
		sender->Cmd.State = comstSendOK;
		return TRUE;
	}
	else
	{
		if (transmissionStatus == TM_NRF24L01_Transmit_Status_Lost)
		{
			msgn("TRANSMIT LOST");
		}
		else
		{
			msgn("UNKNOWN TRANSMIT STATUS");
		}
		sender->Cmd.State = comstSendError;
		return FALSE;
	}
}

void WaterMeter_Step(void)
{
//	LED1_ON();
	if (TM_NRF24L01_DataReady())
	{
		i64 ticks = (i64) GetTicks();

		LED1_ON();
//		TEST1_ON();
//		TEST2_ON();

		do
		{
			TM_NRF24L01_GetData(rb);

//			msgn(rb);

			ProcessPacket(rb, ticks);
		}
		while (TM_NRF24L01_DataReady());
		LED1_OFF();
//		TEST1_OFF();
//		TEST2_OFF();

		TM_NRF24L01_PowerUpRx();
	}

	// for BEEP, do not working now
//	TestSpecialFrequency();

//	LED1_OFF();
}

