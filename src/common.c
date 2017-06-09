// common.c

#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <stdio.h>

#include "lpc17xx_rtc.h"

#include "common.h"
#include "HAL.h"

#include "UART_DEBUG.h"
#include "core_cm3.h"
#include "sbl_iap.h"

//// �������� � ������
//const u32 UDIN_1[4] =
////{ 0x0C0C0C1B, 0x5354259D, 0x4DC62947, 0xF5000005 };
//{ 0x0808FB1B, 0x5354259D, 0x4DC6111E, 0xF5000001 };
//
//const u32 UDIN_2[4] =
//{ 0x0E0E1515, 0x5354259D, 0x4DC639A0, 0xF5000004 };

//bool AbsoluteEnableDebugUart = false;
bool AbsoluteEnableDebugUart = true;
bool EnableDebugUart = true;

const bool Debug_SYS = true;
const bool Debug_UART = false; //true;
const bool Debug_MEM = false; //true;//
const bool Debug_ADC = true; //false;//true;

bool Enable_ShowADC = false; //true;
bool Enable_ShowDAC = false; //true;
bool DAC_Enabled = false; //true;

const bool Debug_DNS = true;
bool Debug_TCP = false; //true;
bool Debug_PPP = false; //true;
bool Debug_PPP_DETAIL = false; //true;
bool Debug_IP = false; //true;

char VersionStr[] = "Ver: v%u.%02x,\t%s";

char Date[] = __DATE__ " " __TIME__ " GMT+0700";

u8 LastResetReason = 0;

volatile u32 Random32;

u8 GLed_counter = 0;

u8* Unit_ID = NULL;
u32 BoardVersion = 51;

//volatile u8 one_sec = 0;

u32 last_mday = 0;
u32 last_mon = 0;
u32 last_year = 0;

//---------------------------------------------------------------------------
/*
i32 atoi(char* s)
{
	i32 res = 0;
	char c;
	bool neg = false;

	while ( *s == ' ')
	{
		s ++;
	}

	if ( *s == '-')
	{
		neg = true;
		s ++;
	}

	c = *s;
	while ((c >= '0') && (c <= '9'))
	{
		res = res * 10 + (c - '0');
		s ++;
		c = *s;
	}

	return ((neg == true) ? -res : res);
}
*/
// *********************************************************************************

//u32 htonl(u32 hostlong){
//	return (ByteSwap4(hostlong)
//);
//}
//
//u16 htons(u16 hostshort){
//	return (ByteSwap2(hostshort)
//);
//}
//
//u32 ntohl(u32 netlong){
//	return (htonl(netlong)
//);
//}
//
//u16 ntohs(u16 netshort){
//	return (htons(netshort));
//}

void str_rtrim(char* s, u8 c)
{
	u32 i, j;
	char* p1;

	if (s == 0)
		return;
	if (c == 0)
		return;
	if ( *s == 0)
		return;

// delete the trailing characters
	if ( *s == 0)
		return;
	j = strlen((char *) s);
	p1 = s + j;
	for (i = 0; i < j; i ++)
	{
		p1 --;
		if ( *p1 != c)
			break;
	}
	if (i < j)
		p1 ++;
	*p1 = 0; // null terminate the undesired trailing characters
}

void str_ltrim(u8 *s, u8 c)
{
	u8 i, *p1, *p2;

	if (s == 0)
		return;

// delete the leading characters
	p1 = s;
	if ( *p1 == 0)
		return;
	for (i = 0; *p1 ++ == c; i ++)
		;
	if (i > 0)
	{
		p2 = s;
		p1 --;
		for (; *p1 != 0;)
			*p2 ++ = *p1 ++;
		*p2 = 0;
	}
}

void strtrim(char* s, u8 c)
{
	u32 i;
	char* p1;
	char* p2;

	if (s == 0)
		return;
	str_rtrim(s, c);

// delete the leading characters
	p1 = s;
	if ( *p1 == 0)
		return;
	for (i = 0; *p1 ++ == c; i ++)
		;
	if (i > 0)
	{
		p2 = s;
		p1 --;
		for (; *p1 != 0;)
			*p2 ++ = *p1 ++;
		*p2 = 0;
	}
}

// **************************************************************************

i32 minint32(i32 a, i32 b)
{
	return (a < b ? a : b);
}
i32 maxint32(i32 a, i32 b)
{
	return (a > b ? a : b);
}

// *********************************************************************************

// ����� ������������ ������ � ������������� �����
char* untosl(char* buf, u32 n)
{
	u8 r;
	char* res;
	u32 nn = n;

	if (n == 0)
	{
		*buf ++ = '0';
	}
	else
	{
		while (nn != 0)
		{
			nn = nn / 10;
			buf ++;
		}

		res = buf - 1;

		while (n != 0)
		{
			r = n % 10;
			*res -- = r + '0';
			n = n / 10;
		}
	}

	*buf = 0x0;

	return (buf);
}

// ����� ������������ uint64 � ������������� �����
char* uint64tosl(char* buf, i64 n)
{
	u8 r;
	char* res;
	i64 nn = n;

	if (n == 0)
	{
		*buf ++ = '0';
	}
	else
	{
		while (nn != 0)
		{
			nn = nn / 10;
			buf ++;
		}

		res = buf - 1;

		while (n != 0)
		{
			r = n % 10;
			*res -- = r + '0';
			n = n / 10;
		}
	}

	*buf = 0x0;

	return (buf);
}

void CR(void)
{
	msgn("");
}

void TAB(void)
{
	msg("\t");
}

u32 CRC = 0;

void BA(void)
{
	msg("<");
	CRC = 0;
}

void EA(void)
{
	msg(">");
	CR();
}

char HexChar[16] =
{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void PutHex(u8 c)
{
	char buf[3] = "__";
	buf[0] = HexChar[(c >> 4) & 0x0F];
	buf[1] = HexChar[c & 0x0F];

	msg(buf);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void dbg(const char* s)
{
	msgn(s);
}

void p64(u64 n)
{
	void __p64(u64 k)
	{
		if (k != (u64) 0)
		{
			__p64(k / (u64) 10);
			putchar(k % (u64) 10 + '0');
		}
	}

	if (n == (u64) 0)
	{
		putchar('0');
	}
	else
	{
		__p64(n);
	}
}

void msg(const char* s)
{
	printf("%s", s);

//	char* buf = s;
//	while(*buf)
//	{
//		putchar(*buf++);
//	}
}

void msgn(const char* s)
{
	printf("%s\n", s);

//	msg(s);
//	putchar('\n');
}

const char HEXTable[16] =
{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void prn8hex(u8 val)
{
	char buf[3] = "  ";
	buf[0] = HEXTable[(val >> 4) & 0x0F];
	buf[1] = HEXTable[val & 0x0F];
	buf[2] = '\0';

	msg(buf);
}

void prn16hex(u16 val)
{
	prn8hex((u8) ((val >> 8) & 0xFF));
	prn8hex((u8) (val & 0xFF));
}

void prn32hex(u32 val)
{
	prn16hex((u16) ((val >> 16) & 0xFFFF));
	prn16hex((u16) (val & 0xFFFF));
}

void prnIP(u32 addr)
{
	u32 n32;
	u8* pa = (u8*) &addr;
	n32 = pa[3];
	printf("%u.", n32);
	n32 = pa[2];
	printf("%u.", n32);
	n32 = pa[1];
	printf("%u.", n32);
	n32 = pa[0];
	printf("%u", n32);
}

void prnMAC(u8* paddr)
{
	printf("%X.%X.%X.%X.%X.%X", (u32) paddr[0], (u32) paddr[1], (u32) paddr[2], (u32) paddr[3], (u32) paddr[4], (u32) paddr[5]);
}

void print_dump(u8* p, u32 len)
{
	for (int y = 0; y < (len / 16 + 1); y ++)
	{
		prn32hex((((u32) p) & 0xFFFFFFF0) + (y * 16));

		msg("\t");

		for (int x = 0; x < 16; x ++)
		{
			prn8hex(p[y * 16 + x]);
			msg(" ");
		}

		msg("\t");

		for (int x = 0; x < 16; x ++)
		{
			char c = p[y * 16 + x];
			char* sc = " ";
			sc[0] = c;
			msg((c >= ' ') ? sc : ".");
		}
		CR();
	}

	CR();
	CR();
}

char MsgBuf[MSG_BUF_SIZE];

void p16(u16 n)
{
	printf("%u", (u32) n);
}

void p32(u32 n)
{
	printf("%u", n);
}

void error(char* s)
{
	//TODO
	msgn(s);
}

void Reset(void)
{
	NVIC_SystemReset();
}

void UnknownBoard(char* s)
{
	printf("ERROR: UnknownBoard (%s)\n\rHALTED\n\r", s);
	delay(100);
	while (1)
		;
}

u32 str2ip(const char* str_buf)
{
	u32 a, b, c, d;
	sscanf(str_buf, "%u.%u.%u.%u", &a, &b, &c, &d);
	return (inet_addr(a, b, c, d));
}

void str2mac(const char* str_buf, u8* mac)
{
	u32 a, b, c, d, e, f;

	sscanf((const char*) str_buf, "%x.%x.%x.%x.%x.%x", &a, &b, &c, &d, &e, &f);
	mac[0] = a;
	mac[1] = b;
	mac[2] = c;
	mac[3] = d;
	mac[4] = e;
	mac[5] = f;
}

//// Unique Device Identification Number
//void TestUDIN(void)
//{
//	u32 udin[4];
//	read_device_serial_number(udin);
//	printf("UDIN = ");
//	prn32hex(udin[0]);
//	printf(" ");
//	prn32hex(udin[1]);
//	printf(" ");
//	prn32hex(udin[2]);
//	printf(" ");
//	prn32hex(udin[3]);
//	CR();
//
//	if ((UDIN_1[0] == udin[0]) && (UDIN_1[1] == udin[1]) && (UDIN_1[2] == udin[2]) && (UDIN_1[3] == udin[3]))
//	{
//		UDIN = 1;
//		ExistsFrontPanel = false;
//		ExistsFourthSensor = true;
//		ExistsMeters = true;
//		printf("UDIN1, 4th sensor, 4 meters\n\r");
//		return;
//	}
//
//	if ((UDIN_2[0] == udin[0]) && (UDIN_2[1] == udin[1]) && (UDIN_2[2] == udin[2]) && (UDIN_2[3] == udin[3]))
//	{
//		UDIN = 2;
//		ExistsFrontPanel = true;
//		ExistsFourthSensor = false;
//		ExistsMeters = false;
//		printf("UDIN2, FrontPanel\n\r");
//		return;
//	}
//
//	UnknownBoard("TestUDIN");
//}

void SaveBootLoaderIP(u32 ip)
{
	RTC_WriteGPREG(LPC_RTC, 0, ip);
	RTC_WriteGPREG(LPC_RTC, 1, ip ^ 0xFFFFFFFF);
}

u32 LoadBootLoaderIP(void)
{
	u32 ip = RTC_ReadGPREG(LPC_RTC, 0);
	u32 ipinv = 0xFFFFFFFF ^ RTC_ReadGPREG(LPC_RTC, 1);

	if (ip == ipinv)
	{
		return (ip);
	}
	else
	{
		return (0);
	}
}

u64 str_to_u64(u8* s)
{
	u64 res = 0;
	while ( *s != '\0')
	{
		u8 n = *s - '0';

		res = res * 10 + n;

		s ++;
	}

	return res;
}

u64 HexToU64(char* s)
{
	u64 res = 0;
	while ( *s != '\0')
	{
		u8 n;
		u8 c = *s;

		if (c <= '9')
		{
			n = c - '0';
		}
		else
		{
			n = c - 'A' + 10;
		}

		res = res * 16 + n;

		s ++;
	}

	return res;
}

void EI(void)
{
	__ASM
volatile ("cpsie i");
}
void DI(void)
{
__ASM
volatile ("cpsid i");
}

void NOP(void)
{
__ASM
volatile ("nop");
}

char* PrnBufStr(char* buf, char* s)
{
	while ( *s != '\0')
	{
		*buf ++ = *s ++;
	}

	*buf = 0;
	return buf;
}

// copy from s to buf up to maxlen chars
char* PrnBufNStr(char* buf, char* s, int maxlen)
{
	int cnt = 0;
	while ( *s != '\0')
	{
		if (cnt >= maxlen)
		{
			break;
		}
		*buf ++ = *s ++;
		cnt ++;
	}

	*buf = 0;
	return buf;
}

char* PrnBufChar(char* buf, char c)
{
	*buf ++ = c;

	*buf = 0;
	return buf;
}

char* PrnBufHex(char* buf, char c)
{
	*buf ++ = HEXTable[(c >> 4) & 0x0F];
	*buf ++ = HEXTable[c & 0x0F];

	*buf = 0;
	return buf;
}

char* PrnBuf32(char* buf, u32 n)
{
	if (n < 10)
	{
		return PrnBufChar(buf, n + '0');
	}
	buf = PrnBuf32(buf, n / 10);

	buf = PrnBufChar(buf, (n % 10) + '0');

	*buf = 0;
	return buf;
}

char* PrnBufFloat(char* buf, float n)
{
	u32 nn;
	u32 nf;

	if (n < 0)
	{
		buf = PrnBufChar(buf, '-');
		n = -n;
	}

	nn = (u32) n;
	nf = (u32) (n * 100) - nn * 100;

//	uart32(nn);
	buf = PrnBuf32(buf, nn);

//	uartc('.');
	buf = PrnBufChar(buf, '.');

	if (nf < 10)
	{
//		uartc('0');
		buf = PrnBufChar(buf, '0');
	}

	buf = PrnBuf32(buf, nf);

	*buf = 0;
	return PrnBuf32(buf, nf);
}

char* PrnBufHex32(char* buf, u32 n)
{
	buf = PrnBufHex(buf, (char) (n >> 24));
	buf = PrnBufHex(buf, (char) (n >> 16));
	buf = PrnBufHex(buf, (char) (n >> 8));
	buf = PrnBufHex(buf, (char) (n));

	return buf;
}

