/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include <arch/memory.h>

/*! stack */
uint8 system_stack [ KERNEL_STACK_SIZE ];

/*! Initialize memory segments (heap) */
mseg_t *arch_memory_init ()
{
	extern char kernel_code_addr, kernel_end_addr;
	mseg_t *mseg;

	mseg = (void *) &kernel_end_addr;

	/* kernel segment - implicitly from kernel linker script */
	mseg[0].type = MS_KERNEL;
	mseg[0].name = "kernel";
	mseg[0].start = &kernel_code_addr;
	mseg[0].size = ( (uint) &kernel_end_addr ) - (uint) &kernel_code_addr
			+ 3 * sizeof (mseg_t);

	/* kernel heap */
	mseg[1].type = MS_KHEAP;
	mseg[1].name = "heap";
	mseg[1].start = &kernel_end_addr + 3 * sizeof (mseg_t);
	mseg[1].size = ( QEMU_MEM << 20 ) - (uint) mseg[1].start;

	mseg[2].type = MS_END;

	return mseg;
}
