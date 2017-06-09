// etc.c

#include <stdio.h>

#include "types.h"
#include "etc.h"
#include "time.h"
#include "common.h"
#include "HAL.h"

void Wellcome(void)
{
	printf("\n\n\n\n\n\n\n");
	printf("*************************\n");
	printf("**    *     *     *    **\n");
	printf("**  SUPER DUMMY UNITS  **\n");
	printf("**     v0.000004(4)    **\n");
	printf("**                     **\n");
	printf("*************************\n");

	printf("SystemCoreClock = %u\n", SystemCoreClock);
}

void ShowCurrDate(void)
{
	time_t rtc_t = GetRTC();
	tm tmr;
	gmtime_x(&rtc_t, &tmr);

	printf("DATE = \"%02u/%02u/%02u,%02u:%02u:%02u+00\"\n", tmr.tm_year - 100, tmr.tm_mon + 1,
	        tmr.tm_mday, tmr.tm_hour, tmr.tm_min, tmr.tm_sec);
}

typedef struct
{
	char* Name;
	u32 ID;
} PartIDStruct;

const PartIDStruct PartID[] =
{
{ "LPC1769", 0x26113F37 },
{ "LPC1768", 0x26013F37 },
{ "LPC1767", 0x26012837 },
{ "LPC1766", 0x26013F33 },
{ "LPC1765", 0x26013733 },
{ "LPC1764", 0x26011922 },
{ "LPC1759", 0x25113737 },
{ "LPC1758", 0x25013F37 },
{ "LPC1756", 0x25011723 },
{ "LPC1754", 0x25011722 },
{ "LPC1752", 0x25001121 },
{ "LPC1751", 0x25001118 } };

#define IAP_LOCATION 0x1FFF1FF1

#define IAP_CMD_PrepareSectors				50
#define IAP_CMD_Copy_RAM_To_Flash 			51
#define IAP_CMD_Erase_Sectors				52
#define IAP_CMD_Blank_Check_Sectors			53
#define IAP_CMD_Read_Part_ID				54
#define IAP_CMD_Read_Boot_Code_Version		55
#define IAP_CMD_Read_Device_Serial_Number	58
#define IAP_CMD_Compare						56
#define IAP_CMD_Reinvoke_ISP				57

//Define data structure or pointers to pass IAP command table and result table to the IAP
//function:
u32 command[5];
u32 result[5];

//Define pointer to function type, which takes two parameters and returns void. Note the IAP
//returns the result with the base address of the table residing in R1.
typedef void (*IAP)(unsigned int[], unsigned int[]);
//Setting function pointer:
static IAP iap_entry = (IAP) IAP_LOCATION;

char* GetPartID(void)
{
	command[0] = IAP_CMD_Read_Part_ID;
	iap_entry(command, result);

	u32 id = result[1];
	char* res = "UNKNOWN";
	for (u32 i = 0; i < (sizeof(PartID) / sizeof(PartIDStruct)); i++)
	{
		if (PartID[i].ID == id)
		{
			res = PartID[i].Name;
		}
	}

	printf("Part_ID = %08X", result[1]);
	return res;
}

void EndianTest(void)
{
	u32 x = 0x01020304;
	u8* p = (u8*) &x;

	u16 a = p[0];

	if (a == (u8) x)
	{
		printf("%s", "LITTLE ENDIAN\n");
	}
	else
	{
		printf("%s", "BIG ENDIAN\n");
	}
}

