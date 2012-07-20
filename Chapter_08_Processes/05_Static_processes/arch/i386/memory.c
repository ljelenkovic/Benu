/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include <arch/memory.h>
#include "descriptor.h"
#include "boot/multiboot.h"
#include <arch/processor.h>
#include <kernel/memory.h>
#include <kernel/kprint.h>

/*! kernel (interrupt) stack */
uint8 k_stack [ KERNEL_STACK_SIZE ];

/*! Multiboot magic number and address where multiboot structure is saved */
unsigned long arch_mb_magic, arch_mb_info;

#define ALIGN_TO	4096 /* align segments to it */

/*! Set up context (normal and interrupt=kernel) */
mseg_t *arch_memory_init ()
{
	extern char kernel_code, k_kernel_end;
	multiboot_info_t *mbi;
	multiboot_module_t *mod;
	mseg_t *mseg;
	uint max;
	int i, j;

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (arch_mb_magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		kprintf ( "Boot loader is not multiboot-compliant!\n" );
		halt();
	}

	/* from multiboot info */
	mbi = (void *) arch_mb_info;

	max = (uint) &k_kernel_end;

	/* first run on modules - check size */
	if ( mbi->flags & MULTIBOOT_INFO_MODS )
	{
		mod = (void *) mbi->mods_addr;
		for ( i = 0; i < mbi->mods_count; i++, mod++ )
			if ( max < mod->mod_end )
				max = mod->mod_end;
	}

	if ( max % ALIGN_TO )
		max += ALIGN_TO - ( max % ALIGN_TO );

	/* reserve space for segment descriptors */
	mseg = (void *) max;
	max += ( 2 + mbi->mods_count + 2 ) * sizeof (mseg_t);

	/* kernel segment - implicitly from kernel linker script */
	mseg[0].type = MS_KERNEL;
	mseg[0].name = NULL;
	mseg[0].start = &kernel_code;
	mseg[0].size = ( (uint) &k_kernel_end ) - (uint) &kernel_code;
	/* modules */
	mseg[1].type = MS_MODULE;
	mseg[1].name = NULL;
	mseg[1].start = ( (void *) &k_kernel_end ) + 1;
	mseg[1].size = max - (uint) &k_kernel_end ;
	/* kernel heap */
	mseg[2].type = MS_KHEAP;
	mseg[2].name = NULL;
	mseg[2].start = (void *) max;
	mseg[2].size = mbi->mem_upper * 1024 - (max - 0x100000);

	/* limit access to range: [ 0, mseg[2].start + mseg[2].size ] */
	arch_update_segments ( NULL, max + mseg[2].size, PRIV_KERNEL);

	/* second run on modules - add each as separate segment */
	j = 3;
	if ( mbi->flags & MULTIBOOT_INFO_MODS )
	{
		mod = (void *) mbi->mods_addr;

		for ( i = 0; i < mbi->mods_count; i++, mod++, j++ )
		{
			mseg[j].type = MS_MODULE;
			mseg[j].name = (char *) mod->cmdline;
			mseg[j].start = (void *) mod->mod_start;
			mseg[j].size = mod->mod_end - mod->mod_start;
		}
	}

	mseg[j].type = MS_END;

	return mseg;
}
