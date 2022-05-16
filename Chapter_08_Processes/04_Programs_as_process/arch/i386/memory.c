/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include <arch/memory.h>

/*! kernel (interrupt) stack */
uint8 system_stack [ KERNEL_STACK_SIZE ];

/* adjust if more than 16 programs are used */
#define MAX_MEMORY_SEGMENTS	4

/* reserve space for segment descriptors */
static mseg_t mseg[MAX_MEMORY_SEGMENTS];

/* multiboot information - currently not used */
unsigned long arch_mb_magic, arch_mb_info;


/*!
 * Create memory map:
 * - find place for heap
 */
mseg_t *arch_memory_init()
{
	extern char kernel_code_addr, kernel_end_addr;
	uint addr, end = (uint) &kernel_end_addr;
	module_t *mod;
	int i = 0;

	/* assuming memory map:
	 * - kernel: kernel_code_addr -- kernel_end_addr
	 * - (debug sections and loader stuff)
	 * - module: [module_t header] [rest of module]
	 * - (more modules)
	 * - free memory => for heap
	 */

	/* kernel segment - from kernel linker script */
	mseg[i].type = MS_KERNEL;
	mseg[i].start = &kernel_code_addr;
	mseg[i].size = (uint) &kernel_end_addr - (uint) &kernel_code_addr;
	i++;

	for (addr = end; addr < SYSTEM_MEMORY; addr+=4)
	{
		mod = (module_t *) addr;

		if (mod->magic[0] == PMAGIC1 && mod->magic[1] == ~PMAGIC1 &&
			mod->magic[2] == PMAGIC2 && mod->magic[3] == ~PMAGIC2)
		{
			/* found module at addr */
			mseg[i].type = mod->type;
			mseg[i].start = (void *) mod; /* physical address! */
			mseg[i].size = (size_t) mod->end - (size_t) mod->start;
			addr += mseg[i].size;
			end = addr;
			i++;
			break; /* only one program! */
		}
	}

	/* kernel heap */
	mseg[i].type = MS_KHEAP;
	mseg[i].start = (void *) end;
	mseg[i].size = SYSTEM_MEMORY - (uint) mseg[i].start;
	i++;

	mseg[i].type = MS_END;

	return mseg;
}
