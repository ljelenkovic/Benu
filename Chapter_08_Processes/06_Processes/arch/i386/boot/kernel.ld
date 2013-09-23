/*! linker script for memory layout of kernel */

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

	kernel_end_addr = .;

	/DISCARD/ : { *(.comment) } /* gcc info is discarded */
	/DISCARD/ : { *(.eh_frame) } /* not used */
}
