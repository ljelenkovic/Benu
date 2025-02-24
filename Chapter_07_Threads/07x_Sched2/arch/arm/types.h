/*! Basic types, machine dependent */

#pragma once

typedef	char 			arch_int8;
typedef	unsigned char 		arch_uint8;
typedef	short int		arch_int16;
typedef	unsigned short int	arch_uint16;
typedef	int 			arch_int32;
typedef	unsigned int 		arch_uint32;
typedef	unsigned int 		arch_uint;

typedef	long long int		arch_int64;
typedef	unsigned long long int	arch_uint64;

/* integer type with same width as pointers */
typedef unsigned int 		arch_aint; /* sizeof(aint) == sizeof(void *) */

/* processor's 'int' size */
#define __ARCH_WORD_SIZE	32
typedef unsigned int		arch_word_t;
typedef int			arch_sword_t; /* "signed" word */

#include <arch/types.h>