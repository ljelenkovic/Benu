/*! Basic types, machine dependent */
#pragma once

#include <ARCH/types.h>

typedef	arch_int8 		int8;
typedef	arch_uint8	 	uint8;
typedef	arch_int16		int16;
typedef	arch_uint16		uint16;
typedef	arch_int32 		int32;
typedef	arch_uint32	 	uint32;
typedef	arch_uint	 	uint;

typedef	arch_int64		int64;
typedef	arch_uint64		uint64;

/* integer type with same width as pointers */
typedef arch_aint	 	aint; /* sizeof(aint) == sizeof(void *) */

/* processor's 'int' size */
#define __WORD_SIZE		__ARCH_WORD_SIZE
typedef arch_word_t		word_t;
typedef arch_sword_t		sword_t; /* "signed" word */
