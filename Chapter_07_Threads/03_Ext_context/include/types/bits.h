/*! Bit manipulations */
#pragma once

#include <arch/bits.h>

/*! Index of first most significant bit or least significant bit */

#ifdef ARCH_MSB_INDEX
#define msb_index	arch_msb_index
#else
#define REQUIRE_MSB_INDEX
#endif

#ifdef ARCH_LSB_INDEX
#define lsb_index	arch_lsb_index
#else
#define REQUIRE_LSB_INDEX
#endif

#ifdef ARCH_MUL_DIV_32
#define mul_div_32	arch_mul_div_32
//#else
//#define REQUIRE_MUL_DIV_32
#endif

/*! use generic implementations for unimplemented functions in arch layer */
#if	defined(REQUIRE_MSB_INDEX) || \
	defined(REQUIRE_LSB_INDEX) || \
	defined(REQUIRE_MUL_DIV_32)

#define REQUIRE_BITS_GENERIC

#include <types/bits_generic.h>

#endif

/*! Random numbers */
#define RAND_MAX_BITS		16
#define RAND_MAX_BITS_SHIFT	((sizeof(uint) * 8 - RAND_MAX_BITS) / 2)

#define RAND_MAX		((1 << RAND_MAX_BITS) - 1)

static inline uint rand(uint *seed)
{
	*seed = (*seed) * 1103515245 + 12345;

	return ((*seed) >> RAND_MAX_BITS_SHIFT) & RAND_MAX;
}
