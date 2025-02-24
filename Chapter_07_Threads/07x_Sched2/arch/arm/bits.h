/*! Bit manipulation functions */

#include <types/basic.h>

/* define which operations are implemented with hardware support */
#define ARCH_MSB_INDEX
#define ARCH_LSB_INDEX

/*!
 * Returns index of MSB (Most Significant Bit) that is not zero
 * \param num	Unsigned number
 * \return MSB index
 */
static inline uint32 arch_msb_index(uint32 num)
{
	return 31 - __builtin_clz(num);
}

/*!
 * Returns index of LSB (Least Significant Bit) that is not zero
 * \param num	Unsigned number
 * \return LSB index
 */
static inline uint32 arch_lsb_index(uint32 num)
{
	return __builtin_ffs(num) - 1;
}
