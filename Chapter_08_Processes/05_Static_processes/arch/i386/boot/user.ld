/*! simple linker script with memory layout of output file */

/* Its parsed as C before used in linking! */

OUTPUT_FORMAT("binary")

ENTRY(prog_init)

SECTIONS {

	/* preallocate all space that is required at runtime! */
	.user 0:
	{
		user_code = .; /* == 0 */

		/* program/module header */
		* ( *.program_header* )

		/* instructions */
		* (.text*)

		user_data = .;

		/* read only data (constants), initialized global variables */
		* ( .rodata* .data* )

		user_bss = .;

		/* uninitialized global variables (or initialized with 0) */
		* ( .bss* COMMON* )

		. = ALIGN (4096);

		user_heap = .;
		. += 0x10000; /* allocate space for heap */

		user_stack = .;
		. += 0x10000; /* allocate space for stack */

		/*
		 * what with other sections generated with gcc (various versions)?
		 * if optimizing for size discard them with: /DISCARD/ : { *(*) }
		 * if debugging with qemu/gdb they must be included
		 * - they can be included implicitly, without declaring them here
		 * - but they will be put behind "kernel_end_addr" so its best to use
		 *   multiboot loader (QEMU is that)
		 */

		user_end = .;
	}

	#ifndef DEBUG
		/DISCARD/ : { *(*) }
	#endif
	/DISCARD/ : { *(.comment*) } /* gcc info is discarded */
	/DISCARD/ : { *(.eh_frame*) } /* not used */
	/DISCARD/ : { *(.note*) } /* not used */
}
