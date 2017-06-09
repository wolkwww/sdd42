// types.h

#ifndef _TYPES_H_
#define _TYPES_H_


#include "lpc_types.h"
#include <stdbool.h>

//typedef boolean bool;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

typedef char		s8;
typedef short		s16;
typedef int			s32;
typedef long long	s64;

typedef char		i8;
typedef short		i16;
typedef int			i32;
typedef long long	i64;

//typedef enum {FALSE = 0, TRUE = !FALSE} Bool;
//typedef Bool bool;

//typedef enum {false = 0, true = !false} bool;

//#define false	(0)
//#define true	(!false);

/*! \brief Unsigned 64-bit value */
//typedef unsigned long long u_longlong;
/*! \brief Void pointer */
typedef void* HANDLE;

#ifndef NULL
#define NULL	((void*)0)
#endif

#ifndef null
#define null	((void*)0)
#endif

#endif
