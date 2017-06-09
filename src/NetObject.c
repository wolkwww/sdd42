// NetObject.c
/*
 *  Created on: 01.08.2013
 *      Author: mrhru
 */

#include "types.h"
#include <stdio.h>

#include "UnitTypes.h"
#include "Utils.h"
#include "Objects.h"
#include "Common.h"
#include "S42Common.h"
#include "Transport.h"
#include "transport_low.h"
#include "config.h"

#include "NetObject.h"

NetInfoDef TmpNetInfo;

static void Proc_LOAD(PObject obj, PVar var);

static void Proc_Main(PObject obj)
{
	// TODO
}

static void Proc_Init(PObject obj)
{
	void CheckAddr(u32* paddr, u32 defaddr)
	{
		if ((*paddr == 0xFFFFFFFF) || (*paddr == 0x00000000))
		{
			*paddr = defaddr;
		}
	}

	void CheckMAC(u8* mac)
	{
		u32 sum = mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5];
		if ((sum == 0) || (sum == 255 * 6))
		{
			u32 udin[4];
			read_device_serial_number(udin);

			mac[0] = 0x42;
			mac[1] = 0x42;
			mac[2] = (u8)(udin[0] >> 24);
			mac[3] = (u8)(udin[0] >> 16);
			mac[4] = (u8)(udin[0] >> 8);
			mac[5] = (u8)(udin[0] >> 0);
		}
	}

	msgn("NET INIT PROC");

	Proc_LOAD(obj, NULL);

	msgn("????????????????????????");

	CheckAddr(&TmpNetInfo.ip_addr, inet_addr(192, 168, 0, 42));
	CheckAddr(&TmpNetInfo.ip_mask, inet_addr(255, 255, 255, 0));
	CheckAddr(&TmpNetInfo.ip_gateway, inet_addr(192, 168, 0, 1));
	CheckMAC(TmpNetInfo.mac_addr);

	msg("IP  :");
	prnIP(TmpNetInfo.ip_addr);
	CR();
	msg("MASK:");
	prnIP(TmpNetInfo.ip_mask);
	CR();
	msg("GATE:");
	prnIP(TmpNetInfo.ip_gateway);
	CR();
	msg("MAC :");
	prnMAC(TmpNetInfo.mac_addr);
	CR();

	memcpy(&NetInfo, &TmpNetInfo, sizeof(NetInfo));

	enc28j60_init(NetInfo.mac_addr);
}

static void SetIP(PObject obj, const char* varname, u32* pip)
{
	char* msg = GetObjVarValue(obj, varname);

	*pip = str2ip(msg);

	printf("NEW %s: \"%s\" ->", varname, msg);
	prnIP(*pip);
	CR();
}

static void Proc_IP(PObject obj, PVar var)
{
	SetIP(obj, "IP", &TmpNetInfo.ip_addr);
}

static void Proc_MAC(PObject obj, PVar var)
{
	char* msg = GetObjVarValue(obj, "MAC");

	str2mac(msg, TmpNetInfo.mac_addr);

	printf("NEW MAC: \"%s\" ->", msg);
	prnMAC(TmpNetInfo.mac_addr);
	CR();
}

static void Proc_MASK(PObject obj, PVar var)
{
	SetIP(obj, "MASK", &TmpNetInfo.ip_mask);
}

static void Proc_GATEWAY(PObject obj, PVar var)
{
	SetIP(obj, "GATEWAY", &TmpNetInfo.ip_gateway);
}

static void Proc_SAVE(PObject obj, PVar var)
{
	SysConfig_Save((u8*) &TmpNetInfo, sizeof(TmpNetInfo));
}

static void Proc_LOAD(PObject obj, PVar var)
{
	msgn("Loading NET config:...");
	SysConfig_Load((u8*) &TmpNetInfo, sizeof(TmpNetInfo));

	msg("IP  :");
	prnIP(TmpNetInfo.ip_addr);
	CR();
	msg("MASK:");
	prnIP(TmpNetInfo.ip_mask);
	CR();
	msg("GATE:");
	prnIP(TmpNetInfo.ip_gateway);
	CR();
	msg("MAC :");
	prnMAC(TmpNetInfo.mac_addr);
	CR();

	msgn("Done...");
}

static const struct TVarDef VarDef[] =
{
{ "IP", Proc_IP, null, 20, "192.168.0.42" },	//
  { "MAC", Proc_MAC, null, 0, "" },	//
  { "MASK", Proc_MASK, null, 20, "255.255.255.0" },	//
  { "GATEWAY", Proc_GATEWAY, null, 20, "192.168.0.1" },	//
  { "SAVE", Proc_SAVE, null, 0, "" },	//
  { "LOAD", Proc_LOAD, null, 0, "" },	//
};

static const struct TEventDef EventDef[] =
{
{ "ON_ERROR", 0 }, //
};

const struct TType TNet =
{ "NET",	// name
  Proc_Main,		//
  Proc_Init,	// InitProc

  null,		//

  (sizeof(VarDef) / sizeof(struct TVarDef)),	//
  VarDef,	//

  (sizeof(EventDef) / sizeof(struct TEventDef)),	//
  EventDef	//
};

typedef struct TNet* PNet;

PObject NetObject;

