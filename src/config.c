// config.c

#include <string.h>
#include <stdio.h>

#include "config.h"
#include "Transport.h"
#include "common.h"
#include "HAL.h"

#include "sbl_config.h"
#include "sbl_iap.h"

// формат хранения:
// string1, 0x00, string2, 0x00, ... stringN, 0x00, 0xFF <- конец

// (256 - мин. размер сектора при записи во flash
// буфер для сектора из flash
#define RAM_BUF_SIZE			256
u8 ram_buf[RAM_BUF_SIZE];

#define FLASH_BUF 				((u8*) (CONFIG_FLASH_START + 0x0000000))

int ramPos; // позиция в озу-буфере
u8* flashPos; // позиция записи во флэш

void Config_ClearAll(void)
{
	memset(ram_buf, 0xFF, RAM_BUF_SIZE);

	ramPos = 0;
	flashPos = FLASH_BUF;

	prepare_sector(CONFIG_FLASH_SECTOR, CONFIG_FLASH_SECTOR);
	erase_sector(CONFIG_FLASH_SECTOR, CONFIG_FLASH_SECTOR);
}

static void addChar(char c)
{
	ram_buf[ramPos] = c;
	ramPos++;

	if (ramPos >= RAM_BUF_SIZE)
	{
		prepare_sector(CONFIG_FLASH_SECTOR, CONFIG_FLASH_SECTOR);
		write_data((u32) flashPos, (u32*) ram_buf, (u32) RAM_BUF_SIZE);
//		print_dump(flashPos, RAM_BUF_SIZE);

		memset(ram_buf, 0xFF, RAM_BUF_SIZE);
		ramPos = 0;
		flashPos += RAM_BUF_SIZE;
	}
}

void Config_AddLine(char* s)
{
	msgn(s);

	u32 len = strlen(s);
	if ((len == 0) || (len >= 254))
	{
		printf("Illegal command line length (%u)\n", len);
		return;
	}

	// копируем строку в буфер
	while (*s)
	{
		addChar(*s++);
	}

	addChar('\0'); //*ram_pos++ = '\0';
	ram_buf[ramPos] = 0xFF;
}

void Config_Save(void)
{
	ram_buf[ramPos] = 0xFF;
	prepare_sector(CONFIG_FLASH_SECTOR, CONFIG_FLASH_SECTOR);
	write_data((u32) flashPos, (u32*) ram_buf, (u32) RAM_BUF_SIZE);
//	print_dump(flashPos, RAM_BUF_SIZE);

	Config_TestShow();
}

//static u8* FindEndRamPos(void)
//{
//	u8* fs = ram_buf;
//	while (*fs != 0xFF)
//	{
//		if (fs >= (u8*) (ram_buf + RAM_BUF_SIZE - 1))
//		{
//			return (u8*) (-1);
//		}
//		fs++;
//	}
//
//	return fs;
//}

char* Config_GetLine(u32 cnt)
{
	u8* s = FLASH_BUF;

	if (*s == 0xFF) return (NULL);

	for (u32 i = 0; i < cnt; i++)
	{
		if (*s == 0xFF) return (NULL);

		while (*s)
		{
			if (s > ((u8*) CONFIG_FLASH_END))
			{
				return (NULL);
			}
			s++;
		}
		s++; // skip zero
	}

	if (*s == 0xFF) return (NULL);
	return (char*) (s);
}

void Config_TestShow(void)
{
	printf("---------------------------------------------\n");
//	print_dump((u8*) CONFIG_FLASH_START, 2048 + 256);

	s32 i = 0;
	char* s = Config_GetLine(i);
	while (s != NULL)
	{
		printf("%u:\t(%u)", i, strlen(s));
		if ((*s != 0xFF) && (strlen(s) < 255))
		{
			printf("<%s>\n", s);
			i++;
			s = Config_GetLine(i);
		}
		else
		{
			printf("Illegal config string\n");
			break;
		}
	}
	printf("---------------------------------------------\n");
}

void PlayConfig(void)
{
	char* s;
	u32 i = 0;

	printf("----------- AUTOLOAD ------------------------\n");

	s = Config_GetLine(i);
	while (s != NULL)
	{
		printf("%u:\t<%s>\n", i, s);
		LocalReceiveMsgAndRun2(s);
		i++;
		s = Config_GetLine(i);

		WDTReset();
	}
}

void TestFlashSave(int n)
{
	for (int i = 0; i < 4096; i++)
	{
		ram_buf[i] = (u8) (i + n);
	}

	prepare_sector(CONFIG_FLASH_SECTOR, CONFIG_FLASH_SECTOR);
	erase_sector(CONFIG_FLASH_SECTOR, CONFIG_FLASH_SECTOR);

	for (int i = 0; i < (4096 / 256); i++)
	{
		prepare_sector(CONFIG_FLASH_SECTOR, CONFIG_FLASH_SECTOR);
		write_data(CONFIG_FLASH_START + i * 256, (u32*) (ram_buf + i * 256), 512);
	}

	print_dump((u8*) CONFIG_FLASH_START, 512);

	for (int i = 0; i < 4096; i++)
	{
		if ((u8) (i + n) != ((u8*) CONFIG_FLASH_START)[i])
		{
			printf("buffer content don't match! %02X-%02X (%u)\n", (u32) ram_buf[i],
			        (u32) (((u8*) CONFIG_FLASH_START)[i]), i);
		}
	}
}


#define SYS_CONFIG_FLASH_SECTOR		28

#define SYS_CONFIG_FLASH_START		sector_start_map[SYS_CONFIG_FLASH_SECTOR]
#define SYS_CONFIG_FLASH_END		sector_end_map[SYS_CONFIG_FLASH_SECTOR]

#define SYS_FLASH_BUF 				((u8*) (SYS_CONFIG_FLASH_START + 0x0000000))

void SysConfig_Save(u8* buf, u16 len)
{
	prepare_sector(SYS_CONFIG_FLASH_SECTOR, SYS_CONFIG_FLASH_SECTOR);
	erase_sector(SYS_CONFIG_FLASH_SECTOR, SYS_CONFIG_FLASH_SECTOR);

	prepare_sector(SYS_CONFIG_FLASH_SECTOR, SYS_CONFIG_FLASH_SECTOR);

	u32 wr_len = min(len, (SYS_CONFIG_FLASH_END - SYS_CONFIG_FLASH_START));
	wr_len = (wr_len & 0xFFFFFF00) + 256;

	write_data((u32)SYS_FLASH_BUF, (u32*) buf, wr_len);

//	print_dump(SYS_FLASH_BUF, len);
}

void SysConfig_Load(u8* buf, u16 len)
{
	for(u32 i = 0; i < len; i++)
	{
		*buf++ = SYS_FLASH_BUF[i];
	}
}
