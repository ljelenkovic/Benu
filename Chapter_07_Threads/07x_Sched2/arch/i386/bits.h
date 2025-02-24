/*! Bit manipulation functions */

#include <types/basic.h>

/* define which operations are implemented with hardware support */
#define ARCH_MSB_INDEX
#define ARCH_LSB_INDEX
#define ARCH_MUL_DIV_32

/*!
 * Returns index of MSB (Most Significant Bit) that is not zero
 * \param num	Unsigned number
 * \return MSB index
 */
static inline uint32 arch_msb_index(uint32 num)
{
/*	int r;

	asm volatile ("bsrl %1, %0": "=r" (r): "rm" (num));

	return r;

*/	return 31 - __builtin_clz(num);
}

/*!
 * Returns index of LSB (Least Significant Bit) that is not zero
 * \param num	Unsigned number
 * \return LSB index
 */
static inline uint32 arch_lsb_index(uint32 num)
{
/*	int r;

	asm volatile ("bsfl %1, %0": "=r" (r): "rm" (num));

	return r;
*/	return __builtin_ffs(num) - 1;
}

/*!
 * Calculate a*b/c (all operands are 32 bit unsigned integers)
 * \param a
 * \param b
 * \param c
 * \return (a*b)/c
 */
static inline uint32 arch_mul_div_32(uint32 a, uint32 b, uint32 c)
{
	uint32 result, m1, m0, mod;

	asm ("mull %2":"=a" (m0), "=d" (m1):"rm" (b), "0" (a));
	asm ("divl %2":"=a" (result), "=d" (mod):"rm" (c), "0" (m0), "1" (m1));

	return result; /* could also return remainder in 'mod' if required! */
}
