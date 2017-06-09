//my_mem.c

#include <stdlib.h>
#include <stdio.h>
//#include <malloc.h>

#include <LPC17xx.h>

#include "types.h"
#include "common.h"
#include "log.h"
#include "UART_DEBUG.h"
#include "HAL.h"
#include "my_mem.h"

/* This version of struct mallinfo must match the one in
   libc/stdlib/mallocr.c.  */

struct mallinfo {
  size_t arena;    /* total space allocated from system */
  size_t ordblks;  /* number of non-inuse chunks */
  size_t smblks;   /* unused -- always zero */
  size_t hblks;    /* number of mmapped regions */
  size_t hblkhd;   /* total space in mmapped regions */
  size_t usmblks;  /* unused -- always zero */
  size_t fsmblks;  /* unused -- always zero */
  size_t uordblks; /* total allocated space */
  size_t fordblks; /* total non-inuse space */
  size_t keepcost; /* top-most, releasable (via malloc_trim) space */
};


extern void* malloc();
extern void free(void*);
extern struct mallinfo mallinfo();

// HEAP size � STACK size ������� � startup_LPC17xx.s

//static void dbg(char* s)
//{
//	if (Debug_MEM)
//	{
//		msg(s);
//	}
//}

static char memTempPad[100];

const u32 ErrorDelay = 5000;

// ������ ���������� ������, ���������� �� ����� Debug_MEM
static void mem_error(u32 size, char* var_name, char* func_name)
{
	printf("\tERROR: can't %s size (%u) for %s\n", func_name, size, var_name);
	delay(ErrorDelay);
}

typedef struct  {
	u32 total;    // total space allocated from system
	u32 inuse; 	// total allocated space
	u32 free; 	// total non-inuse space
} heapinfo;

void get_mem_info(heapinfo* hi)
{
	struct mallinfo mi = mallinfo();
	hi->total = mi.arena;
	hi->inuse = mi.uordblks;
	hi->free = mi.fordblks;
}

void* my_malloc(u32 size, char *var_name)
{
	void* res = malloc(size);

	heapinfo hi;
	get_mem_info(&hi);

	snprintf(_t(memTempPad), "MALLOC %s\tat 0x%X size = %u\t(USED %u OF %u)", var_name, (u32) res, size, hi.inuse, hi.total);
	msgn(memTempPad);
	if (!res)
	{
		mem_error(size, var_name, "MALLOC");
	}
	return (res);
}

void* my_realloc(void* p, u32 size, char *var_name)
{
	void* res = realloc(p, size);

	heapinfo hi;
	get_mem_info(&hi);

	snprintf(_t(memTempPad), "REALLOC %s\tat 0x%X size = %u\t(USED %u OF %u)", var_name, (u32) res, size, hi.inuse, hi.total);
	msgn(memTempPad);
	if (!res)
	{
		mem_error(size, var_name, "REALLOC");
	}
	return (res);
}

#define LPC_RAM_SIZE		(32*1024)

void my_free(void* p, char* var_name)
{
	if ((p >= (void*) LPC_RAM_BASE) && (p < (void*) (LPC_RAM_BASE + LPC_RAM_SIZE)))
	{
		free(p);
	}
	else
	{
		//do nothing
	}

	heapinfo hi;
	get_mem_info(&hi);

	snprintf(_t(memTempPad), "FREE %s\tAT 0x%X\t(USED %u OF %u)", var_name, (u32) p, hi.inuse, hi.total);
	msgn(memTempPad);

}

//////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	void* head;
	u32 freesize;
} MemBlock;

#define LPC_AHBRAM0_BASE (LPC_AHB_BASE)
#define LPC_AHBRAM1_BASE (0x2007c000)

MemBlock MEM2 =
{ (void*) LPC_AHBRAM0_BASE, 0x4000 };
MemBlock MEM1 =
{ (void*) LPC_AHBRAM1_BASE, 0x4000 };


static void* get_mem(MemBlock* mem, u32 size)
{
	u32 tmp_size = ((size / 4) + 2) * 4;
	if (tmp_size > mem->freesize)
	{
		return (NULL);
	}
	else
	{
		void* res = mem->head;
		mem->head += tmp_size;
		mem->freesize -= tmp_size;
		return (res);
	}
}

void* fix_malloc_WO_msg(u32 size)
{
	void* res = NULL;
	if ((res = get_mem(&MEM1, size)) == NULL)
	{
		res = get_mem(&MEM2, size);
	}
	return (res);
}

void* fix_malloc(u32 size, char *var_name)
{
	void* res = fix_malloc_WO_msg(size);

	//	char tmp[100];
	snprintf(_t(memTempPad), "FIX_ALLOC %s\tat 0x%X\tsize = %u,\tFREE MEM = %u", var_name, (u32) res, size,
	        MEM1.freesize + MEM2.freesize);
	msgn(memTempPad);
	if (!res)
	{
		mem_error(size, var_name, "FIX_ALLOC");
	}
	return (res);
}

