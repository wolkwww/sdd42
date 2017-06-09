//common.h

/*
 #include "common.h"
 */

#ifndef _COMMON_H_
#define _COMMON_H_

//#include <stdio.h>

#include "types.h"
#include "time.h"
#include "lpc17xx_gpio.h"

#define METER_COUNT					4

extern int push_cmd_cnt;

extern bool AbsoluteEnableDebugUart;
extern bool EnableDebugUart;

//BOARD1	4-th sensor & 4 meters
//UDIN = 0C0C0C1B 5354259D 4DC62947 F5000005
extern const u32 UDIN_1[4];

//BOARD2	Front panel & PUMP
//UDIN = 0E0E1515 5354259D 4DC639A0 F5000004
extern const u32 UDIN_2[4];

extern u32 UDIN;

extern bool ExistsFrontPanel;
extern bool ExistsFourthSensor;
extern bool ExistsMeters;

extern u32 deltaSysTickCntMax;

void UnknownBoard(char* s);

// Unique Device Identification Number
void TestUDIN(void);

// *********************************************************************************

typedef struct _CurrentStateStruct
{
	float Power;
	float Bat;
} CurrentStateStruct;

extern CurrentStateStruct CurrentState;

// *********************************************************************************

typedef struct _time_ex
{
	time_t sec __attribute__((packed));
	u8 msec; //__attribute__((packed)); // div 10
} time_ex;

#define _t(x)		(char*) x, sizeof(x)

//#TODO understand about real DEV_ID
//DEVID = 0xbd06cb79
#define DEVID 			0xbd06cb7A
#define SWVERSIONMINOR	0U
#define SWVERSIONMAJOR	4U

#define Debug				// comment this out to compile without the debug info (debug text sent to the console)
#define Debug_LOG

extern const bool Debug_SYS;
extern const bool Debug_UART;
extern const bool Debug_AT;
extern const bool Debug_MEM;
extern const bool Debug_ADC;
extern const bool Debug_DNS;

extern bool Debug_IP;
extern u32 Debug_FTP; //1

extern bool DAC_Enabled;

//#define IncludeUDP										// include the UDP code
//#define IncludeDNS										// include the DNS code
//#define IncludeTCP										// include the TCP code

// *********************************************************************************
/*
 * Significant octet values.
 */
#define AHDLC_ALLSTATIONS 0xff    /*!< \brief All-Stations broadcast address */
#define AHDLC_UI          0x03    /*!< \brief Unnumbered Information */
#define AHDLC_FLAG        0x7e    /*!< \brief Flag Sequence */
#define AHDLC_ESCAPE      0x7d    /*!< \brief Asynchronous Control Escape */
#define AHDLC_TRANS       0x20    /*!< \brief Asynchronous transparency modifier */

/*
 * Values for FCS calculations.
 */
#define AHDLC_INITFCS     0xffff  /*!< \brief Initial FCS value */
#define AHDLC_GOODFCS     0xf0b8  /*!< \brief Good final FCS value */

// *********************************************************************************
#define MIN(a, b)			(((a) < (b)) ? (a) : (b))
#define _BV(n)				(1 << (n))
#define bit_is_set(n, b) 	(((n) & (_BV(b))) != 0)
#define bit_is_clear(n, b) 	(((n) & (_BV(b))) == 0)

// *********************************************************************************

#define Disable_Ints()	cli()						// disable global interrupts
#define Enable_Ints()	sei()						// enable global interrupts
#define swap2(val)  (((val & 0xff) << 8) | ((val & 0xff00) >> 8))

#define swap4(val)	(((val & 0x000000ff) << 24) | ((val & 0x0000ff00) << 8) | ((val & 0x00ff0000) >> 8) | ((val & 0xff000000) >> 24))

#define Reset_WD() WDT_Reset()

// *********************************************************************************

extern char VersionStr[];
extern char Date[];

extern u8 LastResetReason;
extern volatile u32 ADC_cnt;

extern volatile u8 Flags1;
extern u8 Flags2;

extern volatile u32 Random32;

extern u32 last_mday;
extern u32 last_mon;
extern u32 last_year;

extern u8 *Unit_ID;

extern u32 BoardVersion;
extern u16 ConnInterval;
extern u16 ConnIntervalEx;

i32 minint32(i32 a, i32 b);
i32 maxint32(i32 a, i32 b);

bool _SendDebugStr(const char* s, u8 code);

s32 CopyToRingBuffer(u8* Dest, u8* Src, s32 idx, s32 BufSize, s32 Bytes);
s32 CopyFromRingBuffer(u8* Dest, u8* Src, s32 idx, s32 BufSize, s32 Bytes);
u32 RingBufBytesFree(u32 BufSize, u32 Rd, u32 Wr);
u32 RingBufBytes(u32 BufSize, u32 Rd, u32 Wr);

//u32 htonl(u32 hostlong);
//u16 htons(u16 hostshort);
//u32 ntohl(u32 netlong);
//u16 ntohs(u16 netshort);

u32 IP_Str(char* buf, u32 IP);
//bool str2ip(char* ipstr, u32 *ipout);

void str_rtrim(char* s, u8 c);
void str_ltrim(u8 *s, u8 c);
void strtrim(char* s, u8 c);

char find_phone(char* line);

u8 EPROMRead(u16 Addr);

u32 get_fattime(void);

u64 str_to_u64(u8* s);
u64 HexToU64(char* s);
// *********************************************************************************

void BA(void);
void EA(void);

void CR(void);
void TAB(void);
void Show(char* msg);
//void BeginAnswer(char* AnswerName);
//void EndAnswer(void);
void ShowError(char* msg);
void ShowUnknownCmd(void);
void ShowInvalidParam(char* msg);
void ShowOK(void);
void Show_Param(char* ParamName, char* ParamValue);
void Show_UINT_Param(char* ParamName, u32 ParamValue);
void Show_UINT64_Param(char* ParamName, u64 ParamValue);
void Show_BOOL_Param(char* ParamName, bool ParamValue);
void Show_FLOAT_Param(char* ParamName, float ParamValue);
void ShowResult(char* res);
void EndAnswer(void);
void PutHex(u8 c);

void msg(const char* s);
void msgn(const char* s);
void dbg(const char* s);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern volatile u16 ms_count;
extern volatile u32 second_count;
extern volatile u16 TimingDelay;

u16 GetMS(void);

// BE conversion

//#define htons(a)			((((a)>>8)&0xff)|(((a)<<8)&0xff00))
//#define ntohs(a)			htons(a)
//
//#define htonl(a)			( (((a)>>24)&0xff) | (((a)>>8)&0xff00) | (((a)<<8)&0xff0000) | (((a)<<24)&0xff000000) )
//#define ntohl(a)			htonl(a)

//#define inet_addr(a,b,c,d)	( ((u32)a) | ((u32)b << 8) | ((u32)c << 16) | ((u32)d << 24) )
#define inet_addr(a,b,c,d)	( ((u32)d) | ((u32)c << 8) | ((u32)b << 16) | ((u32)a << 24) )

void prnIP(u32 addr);
void prnMAC(u8* paddr);

u32 str2ip(const char* str_buf);
void str2mac(const char* str_buf, u8* mac);

void print_dump(u8* p, u32 len);

#define MSG_BUF_SIZE  	512
extern char MsgBuf[MSG_BUF_SIZE];

extern volatile u16 ms_count;
extern volatile u32 second_count;
extern volatile u16 TimingDelay;

u16 GetMS(void);
void CR(void);

void prn8hex(u8 val);
void prn16hex(u16 val);
void prn32hex(u32 val);

void p16(u16 n);
void p32(u32 n);
void p64(u64 n);

// BE conversion

#define min(x, y)			(x < y ? x : y)
#define max(x, y)			(x > y ? x : y)

void error(char* s);
void prefix(const char* s);

void x(int n);

void err(char* s);
void showTime(void);

void EI(void);
void DI(void);
void NOP(void);

void Reset(void);

void SaveBootLoaderIP(u32 ip);
u32 LoadBootLoaderIP(void);

char* PrnBufStr(char* buf, char* s);
// copy from s to buf up to maxlen chars
char* PrnBufNStr(char* buf, char* s, int maxlen);
char* PrnBufChar(char* buf, char c);
char* PrnBufHex(char* buf, char c);
char* PrnBuf32(char* buf, u32 n);
char* PrnBufFloat(char* buf, float n);
char* PrnBufHex32(char* buf, u32 n);


#endif
