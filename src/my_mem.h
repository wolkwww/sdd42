//my_mem.h

/*
#include "my_mem.h"
 */
#ifndef my_mem_h
#define my_mem_h

#include "common.h"

void* fix_malloc(u32 size, char *var_name);

void *my_malloc(u32 size, char* var_name);
void *my_realloc(void *p, u32 size, char* var_name);
void my_free(void *p, char* var_name);

#endif
