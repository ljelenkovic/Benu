/*! simple linker script with memory layout of output file */

OUTPUT_FORMAT("binary")

ENTRY(prog_init)

SECTIONS {

	.user 0x200000: /* prepare for absolute address */
	{
		user_code = .;

		/* header */
		*api/prog_info.o ( *.data* )

		/* instructions */
		* (.text*)

		user_data = .;

		/* read only data (constants), initialized global variables */
		* ( .rodata* .data* )

		user_bss = .;

		/* uninitialized global variables (or initialized with 0) */
		* ( .bss* COMMON* )

		. = ALIGN (4096);

		user_end = .;
	}

	/DISCARD/ : { *(.comment) } /* gcc info is discarded */
	/DISCARD/ : { *(.eh_frame) } /* not used */
}
