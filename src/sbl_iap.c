//sbl_iap.c
//-----------------------------------------------------------------------------
// Software that is described herein is for illustrative purposes only  
// which provides customers with programming information regarding the  
// products. This software is supplied "AS IS" without any warranties.  
// NXP Semiconductors assumes no responsibility or liability for the 
// use of the software, conveys no license or title under any patent, 
// copyright, or mask work right to the product. NXP Semiconductors 
// reserves the right to make changes in the software without 
// notification. NXP Semiconductors also make no representation or 
// warranty that such application will be suitable for the specified 
// use without further testing or modification. 
//-----------------------------------------------------------------------------

#include <stdio.h>

//#include "type.h"
//#include "nvic.h"
#include "sbl_iap.h"
#include "sbl_config.h"
#include "common.h"
#include "HAL.h"

const u32 crp __attribute__((section(".ARM.__at_0x1FC"))) = CRP;

const u32 sector_start_map[MAX_FLASH_SECTOR] =
{ SECTOR_0_START, SECTOR_1_START, SECTOR_2_START, SECTOR_3_START, SECTOR_4_START, SECTOR_5_START, SECTOR_6_START,
  SECTOR_7_START, SECTOR_8_START, SECTOR_9_START, SECTOR_10_START, SECTOR_11_START, SECTOR_12_START, SECTOR_13_START,
  SECTOR_14_START, SECTOR_15_START, SECTOR_16_START, SECTOR_17_START, SECTOR_18_START, SECTOR_19_START, SECTOR_20_START,
  SECTOR_21_START, SECTOR_22_START, SECTOR_23_START, SECTOR_24_START, SECTOR_25_START, SECTOR_26_START, SECTOR_27_START,
  SECTOR_28_START, SECTOR_29_START };

const u32 sector_end_map[MAX_FLASH_SECTOR] =
{ SECTOR_0_END, SECTOR_1_END, SECTOR_2_END, SECTOR_3_END, SECTOR_4_END, SECTOR_5_END, SECTOR_6_END, SECTOR_7_END,
  SECTOR_8_END, SECTOR_9_END, SECTOR_10_END, SECTOR_11_END, SECTOR_12_END, SECTOR_13_END, SECTOR_14_END, SECTOR_15_END,
  SECTOR_16_END, SECTOR_17_END, SECTOR_18_END, SECTOR_19_END, SECTOR_20_END, SECTOR_21_END, SECTOR_22_END,
  SECTOR_23_END, SECTOR_24_END, SECTOR_25_END, SECTOR_26_END, SECTOR_27_END, SECTOR_28_END, SECTOR_29_END };

u32 param_table[5];
u32 result_table[5];

void iap_entry(u32 param_tab[], u32 result_tab[]);

void write_data(u32 flash_address, u32* flash_data_buf, u32 count)
{
	printf("WRITE_DATA: [%x] <- [%x] (%u) \n", flash_address, (u32) flash_data_buf, count);

	DI();

	param_table[0] = COPY_RAM_TO_FLASH;
	param_table[1] = flash_address;
	param_table[2] = (u32) flash_data_buf;
	param_table[3] = count;
	param_table[4] = SystemCoreClock / 1000U;
	iap_entry(param_table, result_table);

	EI();

	if (result_table[0] != CMD_SUCCESS)
	{
		printf("ERROR: write_data (%u) failed.\n\rHALTED\n", flash_address);
		while (1)
			; // No way to recover. Just let Windows report a write failure
	}
}

void erase_sector(u32 start_sector, u32 end_sector)
{
	DI();

	param_table[0] = ERASE_SECTOR;
	param_table[1] = start_sector;
	param_table[2] = end_sector;
	param_table[3] = SystemCoreClock / 1000U;
	;
	iap_entry(param_table, result_table);

	if (result_table[0] != CMD_SUCCESS)
	{
		printf("ERROR: erase_user_flash sector (%u)(%u) failed.\n\rHALTED\n", start_sector, end_sector);
		while (1)
			; // No way to recover. Just let Windows report a write failure
	}

	EI();
}

void prepare_sector(u32 start_sector, u32 end_sector)
{
	param_table[0] = PREPARE_SECTOR_FOR_WRITE;
	param_table[1] = start_sector;
	param_table[2] = end_sector;
	param_table[3] = SystemCoreClock / 1000U;
	;
	iap_entry(param_table, result_table);

	if (result_table[0] != CMD_SUCCESS)
	{
		printf("ERROR: prepare_sectors (%u)(%u) failed.\n\rHALTED\n", start_sector, end_sector);
		while (1)
			; // No way to recover. Just let Windows report a write failure
	}
}

void iap_entry(u32 param_tab[], u32 result_tab[])
{
	void (*iap)(u32[], u32[]);

	iap = (void (*)(u32[], u32[])) IAP_ADDRESS;
	iap(param_tab, result_tab);
}

Bool user_code_present(void)
{
	param_table[0] = BLANK_CHECK_SECTOR;
	param_table[1] = USER_START_SECTOR;
	param_table[2] = USER_START_SECTOR;
	iap_entry(param_table, result_table);
	if (result_table[0] == CMD_SUCCESS)
	{
		return (FALSE);
	}
	else
	{
		return (TRUE);
	}
}

void read_device_serial_number(u32* UDIN)
{
	param_table[0] = READ_DEVICE_SERIAL_NUMBER;
	param_table[1] = 0;
	param_table[2] = 0;
	iap_entry(param_table, result_table);
	if (result_table[0] == CMD_SUCCESS)
	{
		UDIN[0] = result_table[1];
		UDIN[1] = result_table[2];
		UDIN[2] = result_table[3];
		UDIN[3] = result_table[4];
	}
	else
	{
		UDIN[0] = UDIN[1] = UDIN[2] = UDIN[3] = 0;
	}
}
