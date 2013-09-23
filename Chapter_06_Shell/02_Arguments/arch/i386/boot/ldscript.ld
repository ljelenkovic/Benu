/*! linker script for memory layout */

ENTRY(arch_startup)

SECTIONS {
	.code LOAD_ADDR :
	{
		kernel_code = .;

		/* instructions */
		*?/boot/startup.asm.o ( .text* )

		*( .text* )
	}
	.data :
	{
		kernel_data = .;

		/* read only data (constants), initialized global variables */
		* ( .rodata* .data* )
	}
	.bss :
	{
		*( .bss* COMMON* )

		. = ALIGN (4096);
	}

	k_kernel_end = .;

	/DISCARD/ : { *(.comment) } /* gcc info is discarded */
	/DISCARD/ : { *(.eh_frame) } /* not used */
}
