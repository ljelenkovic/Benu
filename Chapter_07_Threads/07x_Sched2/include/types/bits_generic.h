/*! Bit manipulation functions */
#pragma once

/*! DO NOT INCLUDE DIRECTLY! Use <types/bits.h> */

#ifndef REQUIRE_BITS_GENERIC
#error Do not include this file directly!
#endif

#include <types/basic.h>

#ifdef REQUIRE_MSB_INDEX

#if __WORD_SIZE == 32
#define msb_index	msb_index_32
#elif __WORD_SIZE == 64
#define msb_index	msb_index_64
#else
#define msb_index	msb_index_generic
#endif

#endif /* REQUIRE_MSB_INDEX */

#if 0 /* lsb_index and mul_div_32 not implemented here in software mode */
#ifdef REQUIRE_LSB_INDEX

#if __WORD_SIZE == 32
#define lsb_index	lsb_index_32
#elif __WORD_SIZE == 64
#define lsb_index	lsb_index_64
#else
#define lsb_index	lsb_index_generic
#endif

#define mul_div_32	mul_div_32_generic

#endif /* REQUIRE_LSB_INDEX */
#endif

/* All implementation assume num > 0 (functions don't check it) */

/* optimized for 32-bit system */
static inline uint32 msb_index_32(uint32 num)
{
	uint32 msb;

	msb = 0;

	if (num >= (1<<16)) { msb += 16; num >>= 16; }
	if (num >= (1<<8)) { msb += 8;  num >>= 8;  }
	if (num >= (1<<4)) { msb += 4;  num >>= 4;  }
	if (num >= (1<<2)) { msb += 2;  num >>= 2;  }
	if (num >= (1<<1)) { msb += 1;              }

	return msb;
}

/* optimized for 64-bit system */
static inline int msb_index_64(uint64 num)
{
	int msb;

	msb = 0;

	if (num >= (1ULL<<32)) { msb += 32; num >>= 32; }
	if (num >= (1<<16)) { msb += 16; num >>= 16; }
	if (num >= (1<<8)) { msb += 8;  num >>= 8;  }
	if (num >= (1<<4)) { msb += 4;  num >>= 4;  }
	if (num >= (1<<2)) { msb += 2;  num >>= 2;  }
	if (num >= (1<<1)) { msb += 1;              }

	return msb;
}

/* generic function for type 'unsigned int' */
static inline int msb_index_generic(unsigned int num)
{
	unsigned int half;
	int i, msb;

	msb = 0;
	for (i = sizeof(unsigned int) * 8 / 2; i > 0; i >>= 1)
	{
		half = ((unsigned int) 1) << i;
		if (num >= half)
		{
			num >>= i;
			msb += i;
		}
	}
	return msb;
}


/* generic function for type required integer type */
typedef unsigned long long int int_n; /* change type as required */

static inline int msb_index_int_n(int_n num)
{
	int_n half;
	int i, msb;

	msb = 0;
	for (i = sizeof(int_n) * 8 / 2; i > 0; i >>= 1)
	{
		half = ((int_n) 1) << i;
		if (num >= half)
		{
			num >>= i;
			msb += i;
		}
	}
	return msb;
}
