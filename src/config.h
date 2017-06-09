// config.h

#ifndef _CONFIG_H
#define _CONFIG_H

#include "types.h"
#include "sbl_iap.h"

// for 512K device
#define CONFIG_FLASH_SECTOR		27

#define CONFIG_FLASH_START		sector_start_map[CONFIG_FLASH_SECTOR]
#define CONFIG_FLASH_END		sector_end_map[CONFIG_FLASH_SECTOR]

void Config_ClearAll(void);
void Config_AddLine(char* s);
void Config_Save(void);

char* Config_GetLine(u32 cnt);
void PlayConfig(void);

void Config_TestShow(void);

// SYSTEM CONFIG

// for 512K device
#define SYS_CONFIG_FLASH_SECTOR		28

#define SYS_CONFIG_FLASH_START		sector_start_map[SYS_CONFIG_FLASH_SECTOR]
#define SYS_CONFIG_FLASH_END		sector_end_map[SYS_CONFIG_FLASH_SECTOR]

void SysConfig_Save(u8* buf, u16 len);
void SysConfig_Load(u8* buf, u16 len);

#endif

