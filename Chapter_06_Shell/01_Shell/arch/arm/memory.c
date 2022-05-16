/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include <arch/memory.h>

/*! kernel (interrupt) stack */
uint8 system_stack [ STACK_SIZE ];

/* reserve space for segment descriptors */
static mseg_t mseg[3];

/*!
 * Create memory map:
 * - find where to place heap
 */
mseg_t *arch_memory_init()
{
	extern char kernel_code_addr, kernel_end_addr;

	/* kernel segment - from kernel linker script */
	mseg[0].type = MS_KERNEL;
	mseg[0].start = &kernel_code_addr;
	mseg[0].size = (uint) &kernel_end_addr - (uint) &kernel_code_addr;

	/* kernel heap */
	mseg[1].type = MS_KHEAP;
	mseg[1].start = &kernel_end_addr;
	mseg[1].size = SYSTEM_MEMORY - (uint) mseg[1].start;

	mseg[2].type = MS_END;

	return mseg;
}
