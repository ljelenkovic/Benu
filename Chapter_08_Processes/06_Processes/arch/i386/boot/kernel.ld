/*! linker script for memory layout of kernel */

/* OUTPUT_FORMAT("elf32-i386") */

ENTRY(arch_startup)

SECTIONS {
	.kernel LOAD_ADDR :
	{
		kernel_code = .;

		*?/ARCH/boot/startup.asm.o ( .text* )
		*( .text* )

		/* *( EXCLUDE_FILE ( *?/ARCH/syscall.asm.o *?/programs/?* )
			.text* )*/

		kernel_data = .;

		*( .rodata* .data* )
		/* *( EXCLUDE_FILE ( *?/ARCH/syscall.asm.o *?/programs/?* )
			.rodata* .data* ) */
	}
	.kernel_bss :
	{
		*( .bss* COMMON* )
		/* *( EXCLUDE_FILE ( *?/ARCH/syscall.asm.o *?/programs/?* )
			.bss* COMMON* ) */

		. = ALIGN (4096);
	}

	k_kernel_end = .;

	/DISCARD/ : { *(.comment) } /* gcc info is discarded */
	/DISCARD/ : { *(.eh_frame) } /* not used */
}
