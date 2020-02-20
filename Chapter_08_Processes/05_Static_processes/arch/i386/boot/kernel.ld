/*! linker script for memory layout of kernel */

/* Its parsed as C before used in linking! */

/* OUTPUT_FORMAT("elf32-i386") */

ENTRY(arch_startup)

SECTIONS {
	.kernel LOAD_ADDR :
	{
		kernel_code_addr = .;

		*?/boot/startup.asm.o ( .text* )
		*( .text* )

		/* *( EXCLUDE_FILE ( *?/arch/i386/syscall.asm.o *?/programs/?* )
			.text* )*/

		kernel_data_addr = .;

		*( .rodata* .data* )
		/* *( EXCLUDE_FILE ( *?/arch/i386/syscall.asm.o *?/programs/?* )
			.rodata* .data* ) */
	}
	.kernel_bss :
	{
		*( .bss* COMMON* )
		/* *( EXCLUDE_FILE ( *?/arch/i386/syscall.asm.o *?/programs/?* )
			.bss* COMMON* ) */

		. = ALIGN (4096);
	}

	/*
	 * what with other sections generated with gcc (various versions)?
	 * if optimizing for size discard them with: /DISCARD/ : { *(*) }
	 * if debugging with qemu/gdb they must be included
	 * - they can be included implicitly, without declaring them here
	 * - but they will be put behind "kernel_end_addr" so its best to use
	 *   multiboot loader (QEMU is that)
	 */

#ifndef DEBUG
	/DISCARD/ : { *(*) }
#endif
	kernel_end_addr = .;

	/DISCARD/ : { *(.comment*) } /* gcc info is discarded */
	/DISCARD/ : { *(.eh_frame*) } /* not used */
	/DISCARD/ : { *(.note*) } /* not used */

}
