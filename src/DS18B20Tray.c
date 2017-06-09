/*
 * DS18B20Tray.c
 *
 *  Created on: 26 ма€ 2017 г.
 *      Author: mrhru
 */

#include "defines.h"
#ifdef TERMO_STATION

#include <string.h>
#include <stdio.h>

#include "tm_stm32f3_nrf24l01.h"
#include "HAL.h"
#include "UnitTypes.h"
#include "Utils.h"

#include "DS18B20Tray.h"

#define DS_HUB_ADDRESS					"DSHUB"
#define TRAY_BROADCAST_ADDRESS			"DTRAY"
#define RF_CHANNEL_DS18B20				119

#define CMD_FORGET_COOL_AND_HEAT_TEMPS	"FORGET_TEMPS"
#define CMD_SET_COOL_TEMP				"SET_COOL"
#define CMD_SET_HEAT_TEMP				"SET_HEAT"

#define TRAY_MAX_COUNT			10
#define TRAY_PRESENT_TIMEOUT	10

#define TRAY_UID_LENGTH		(5 + 1)
#define TRAY_AVER_LENGTH	8
#define UID_LENGHT			4

typedef struct
{
	u32 uid;

	bool present;

	float average;
	bool set_cool;
	bool set_warm;

	u32 last_update_time;
} TrayTypeDef;

typedef struct
{
	TrayTypeDef List[TRAY_MAX_COUNT];
} TrayListTypeDef;

TrayListTypeDef Trays;

typedef TrayTypeDef* PTray;

static char HexCharToHalfByte(char hexChar)
{
	if ((hexChar >= '0') && (hexChar <= '9'))
	{
		return hexChar - '0';
	}
	else
	{
		if ((hexChar >= 'A') && (hexChar <= 'F'))
		{
			return hexChar - 'A' + 10;
		}
		else
		{
			return 0;
		}
	}
}

//static u8* UIDToStr(u32 uid, u8* str)
//{
//	u8* puid = (u8*) ( &uid);
//
//	u8 i;
//	for (i = 0; i < UID_LENGHT; i ++)
//	{
//		str = (u8*) PrnBufHex((char*) str, puid[i]);
//	}
//
//	return str;
//}

static u32 StrToUID(char* str)
{
	u32 uid = 0;

	u8 i;
	for (i = 0; i < UID_LENGHT; i ++)
	{
		u8 c = (u8) ((HexCharToHalfByte(str[2 * i]) << 4) | HexCharToHalfByte(str[2 * i + 1]));
		uid = (uid << 8) | c;
	}

	return uid;
}

static void ShowTrays(void)
{
	msgn("--- TRAY LIST ---");

	msgn("UID\t\tpresent\taverage\tcool\twarm\tupdate_time");
	for (int i = 0; i < TRAY_MAX_COUNT; i ++)
	{
		prn32hex(Trays.List[i].uid);

		msg("\t");
		p32(Trays.List[i].present);

		msg("\t");
		p32(Trays.List[i].average);

		msg("\t");
		p32(Trays.List[i].set_cool);

		msg("\t");
		p32(Trays.List[i].set_warm);

		msg("\t");
		p32(Trays.List[i].last_update_time);

		CR();
	}
	msgn("-----------------");
}

/*
 * statNum - номер холодильника
 *
 * "XXXXXXXX/NM:12.34;xx
 * XXXXXXXX строка в HEX, д.б. 8 символов кодирующие 4 байта uid
 * NM - N - номер холодильника 1..9, M - номер полки 1..9
 * A - признак средней температуры
 * 12.34 - температура
 * cw - два символа 0/1 как признаки того, что поддон запомнил
 * температуры холода (c) и тепла (w) соответственно
 */
void UpdateTray(int statNum, char* s)
{
	msgn("UpdateTray\t<");
	msg((char*) s);
	msgn(">");

	char* s_UID = s;
	char* s_StatBrd = strchr(s, '/');
	char* s_TEMP = strchr(s, ':');
	char* s_CoolWarm = strchr(s, ';');

	if ((s_StatBrd != NULL) && (s_TEMP != NULL) && (s_CoolWarm != NULL))
	{
		// указывал на '/'
		*s_StatBrd = '\0';
		s_StatBrd ++;

		// указывал на ':'
		*s_TEMP = '\0';
		s_TEMP ++;

		// указывал на ';'
		*s_CoolWarm = '\0';
		s_CoolWarm ++;

		u32 uid = StrToUID(s_UID);
		int stat = s_StatBrd[0] - '0';
		int trayNum = s_StatBrd[1] - '0';

		float averTemp = ValueAsFloat(s_TEMP);
		u8 cool = s_CoolWarm[0] - '0';
		u8 warm = s_CoolWarm[1] - '0';

		CR();
		msg("UID = ");
		prn32hex(uid);

		msg("\tstat = ");
		p32(stat);

		msg("\tbrd = ");
		p32(trayNum);

		msg("\tAVER = ");
		msg(s_TEMP);

		msg("\tcool = ");
		p16(cool);

		msg("\twarm = ");
		p16(warm);

		CR();

		if ((statNum == stat) && (trayNum >= 1) && (trayNum <= 9))
		{
			int ch = trayNum - 1;
			// обновл€ем поддон
			Trays.List[ch].uid = uid;
			Trays.List[ch].present = true;
			Trays.List[ch].average = averTemp;
			Trays.List[ch].set_cool = cool;
			Trays.List[ch].set_warm = warm;
			Trays.List[ch].last_update_time = GetRTC();
			;
		}
	}
}

static void UpdateTimeouts(void)
{
	u32 leave_time = GetRTC() - TRAY_PRESENT_TIMEOUT;

	for (int idx = 0; idx < TRAY_MAX_COUNT; idx ++)
	{
		if ((Trays.List[idx].last_update_time) < leave_time)
		{
			Trays.List[idx].present = false;
		}
		else
		{
			Trays.List[idx].present = true;
		}
	}
}

static void ClearTrayList(void)
{
	for (int i = 0; i < TRAY_MAX_COUNT; i ++)
	{
		Trays.List[i].uid = 0;
		Trays.List[i].present = false;
		Trays.List[i].average = 0.0;
		Trays.List[i].set_cool = false;
		Trays.List[i].set_warm = false;
		Trays.List[i].last_update_time = 0;
	}
}

u8 dataIn[33];
u8 dataOut[33];
static bool nRF_inited = false;

static void Init_nRF(void)
{
	TM_NRF24L01_Init(RF_CHANNEL_DS18B20, 32, 0);

	TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_2M, TM_NRF24L01_OutputPower_0dBm);
}

void TrayInit(void)
{
	if ( !nRF_inited)
	{
		Init_nRF();
		TM_NRF24L01_SetMyAddress((u8*) DS_HUB_ADDRESS);

		NRF24L01_CE_LOW();
		TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, ((1 << NRF24L01_EN_CRC) | (1 << NRF24L01_CRCO) | (1 << NRF24L01_MASK_TX_DS) | (1 << NRF24L01_MASK_MAX_RT)));
		NRF24L01_CE_HIGH();

		ShowRegs();
		delay(1000); //TODO TEST, REAL 100

		NRF24L01_CSN_LOW();
		NRF24L01_SCK_LOW();

		TM_NRF24L01_PowerDown();
		msgn(" TRAY HUB");
		TM_NRF24L01_PowerUpRx();

		ClearTrayList();

		nRF_inited = true;
	}
}

static void PrepareBuf(char* buf)
{
	PrnBufChar(buf, '\0');

	msg(" <");
	msg((char*) dataOut);
	msgn("> ");
}

static void Send(void)
{
	TM_NRF24L01_PowerUpTx();

	TM_NRF24L01_Transmit(dataOut);

	TM_NRF24L01_Transmit_Status_t transmissionStatus;
	do
	{
		transmissionStatus = TM_NRF24L01_GetTransmissionStatus();
	}
	while (transmissionStatus == TM_NRF24L01_Transmit_Status_Sending);
}

static void SendToTrayByAddr(u8* addr)
{
	msg("SEND ");

	for (int i = 0; i < 5; i ++)
	{
		prn8hex(addr[i]);
	}
	msg(": <");
	msg((char*) dataOut);
	msgn("> ");

	TM_NRF24L01_SetTxAddress(addr);
//	TM_NRF24L01_SetTxAddress((u8*) TRAY_BROADCAST_ADDRESS);

	Send();

	TM_NRF24L01_PowerUpRx();
}

static void SendCmd(int ch, char* msg)
{
	if ((ch >= 1) && (ch <= TRAY_MAX_COUNT))
	{
		ch = ch - 1;

		char* buf = (char*) dataOut;
		buf = PrnBufStr(buf, msg);
		PrepareBuf(buf);

		u8 addrBuf[6];
		buf = (char*) addrBuf;
		buf = PrnBufChar(buf, '_');

		p32(Trays.List[ch].uid);
		CR();

		u8* trayAddr = (u8*) ( &Trays.List[ch].uid);
		for (int i = 3; i >= 0; i --) // инверсный пор€док
		{
			buf = PrnBufChar(buf, trayAddr[i]);
		}

		SendToTrayByAddr(addrBuf);
	}
}

void SendForgetCmd(int ch)
{
	SendCmd(ch, CMD_FORGET_COOL_AND_HEAT_TEMPS);
}
void SendSetCoolCmd(int ch)
{
	SendCmd(ch, CMD_SET_COOL_TEMP);
}
void SendSetWarmCmd(int ch)
{
	SendCmd(ch, CMD_SET_HEAT_TEMP);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TRAY TYPE

static void Proc_FORGET(PObject obj, PVar var)
{
	int ch = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));

	printf("TRAY (%u) FORGET\n\r", ch);
	SendForgetCmd(ch);
}

static void Proc_COOL(PObject obj, PVar var)
{
	int ch = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));

	printf("TRAY (%u) SET COOL TEMP\n\r", ch);
	SendSetCoolCmd(ch);

}

static void Proc_WARM(PObject obj, PVar var)
{
	int ch = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));

	printf("TRAY (%u) SET WARM TEMP\n\r", ch);
	SendSetWarmCmd(ch);
}

static void Proc_INIT(PObject obj)
{
	TrayInit();
}

static void Proc_MAIN(PObject obj)
{
	int statNum = ValueAsInt(GetObjVarValue(obj, "STATION"));
	int ch = ValueAsInt(GetObjVarValue(obj, "CHANNEL"));

// чтение из nRF производим однократно, а не дл€ каждого поддона
	if (ch <= 1)
	{
		while (TM_NRF24L01_DataReady())
		{
			TM_NRF24L01_GetData(dataIn);

			msg("RX: <");
			msg((char*) dataIn);
			msgn(">");

			UpdateTray(statNum, (char*) dataIn);

			TM_NRF24L01_PowerUpRx();
		}

		UpdateTimeouts();

		static u32 last_show_time;
		if(last_show_time != GetRTC())
		{
			last_show_time = GetRTC();
			ShowTrays();
		}
	}

	PTray ptray = &Trays.List[ch - 1];

	PVar var;
	var = FindVar(obj, "PRESENT");

	SetVarAsInt(var, (ptray->present) ? 1 : 0);

	if (ptray->present)
	{
		var = FindVar(obj, "AVERAGE");
		SetVarAsFloat(var, ptray->average);

		var = FindVar(obj, "COOL");
		SetVarAsInt(var, ptray->set_cool);

		var = FindVar(obj, "WARM");
		SetVarAsInt(var, ptray->set_warm);
	}


}

const struct TVarDef TrayVarDef[] =
{
{"STATION", null, null, 2, "-1"}, 	// номер холодильника, 1..
    {"CHANNEL", null, null, 2, "-1"}, 	// номер поддона, 1..

    {"PRESENT", null, null, 2, "0"}, 	// признак присутстви€ поддона
    {"TIMEOUT", null, null, 8, "15"}, 	// таймаут сек. по потере поддона - при отсутствии сигналов от поддона

    {"AVERAGE", null, null, 8, "0"}, 	// средн€€ температура

    {"FORGET", Proc_FORGET, null, 2, "0"}, 	// команда на забывание всех температур
    {"COOL", Proc_COOL, null, 2, "0"}, 		// команда на сохранение темп. холода, на чтение - признак от поддона
    {"WARM", Proc_WARM, null, 2, "0"}, 		// команда на сохранение темп. тепла, на чтение - признак от поддона
    };

const struct TEventDef TrayEventDef[] = {
//{"ON_START", 0},
//{"ON_DONE", 0},
//{"ON_ERROR", 0}
    };

const struct TType TTray =
{"TRAY", 		// name
    Proc_MAIN, 	// main proc
    Proc_INIT,	// InitProc

    null,

    (sizeof(TrayVarDef) / sizeof(struct TVarDef)),
    TrayVarDef,

    (sizeof(TrayEventDef) / sizeof(struct TEventDef)),
    TrayEventDef,
    0};

#endif
