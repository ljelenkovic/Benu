/*! linker script for memory layout */

ENTRY(arch_startup)

ROM_ADDR      = 0x10000;
RAM_ADDR      = 0x50000;
KERN_CODE_RAM = 0x50000;
KERN_DATA_RAM = 0x60000;
PROG_CODE_RAM = 0x70000;
PROG_DATA_RAM = 0x80000;
STACK_ADDR    = 0x90000;

SECTIONS {
	boot_code_ROM = ROM_ADDR;
	.boot_code boot_code_ROM : AT (boot_code_ROM)
	{
		/* runs from ROM */
		build/arch/?*?/boot/?* ( .text* .rodata* .data* .bss*)
	}

	kern_code_ROM = boot_code_ROM + SIZEOF(.boot_code);
	.kern_code KERN_CODE_RAM : AT (kern_code_ROM)
	{
		/* copy to RAM and then run from there */
		build/arch/?* ( .text* )
		build/kernel/?* ( .text* )
		build/lib/?* ( .text* )

		/* or: build/arch/?* build/kernel/?* build/lib/?* (.text*) */
		/* or: *(EXCLUDE_FILE(build/api/?* build/programs/?*) .text*) */
	}
	kern_code_size = SIZEOF(.kern_code);

	kern_data_ROM = kern_code_ROM + kern_code_size;
	.kern_data KERN_DATA_RAM : AT (kern_data_ROM)
	{
		/* copy to RAM and then use from there */
		build/arch/?* ( .rodata* .data* .bss* COMMON* )
		build/kernel/?* ( .rodata* .data* .bss* COMMON*  )
		build/lib/?* ( .rodata* .data* .bss* COMMON*  )
	}
	kern_data_size = SIZEOF(.kern_data);

	prog_code_ROM = kern_data_ROM + kern_data_size;
	prog_code PROG_CODE_RAM : AT (prog_code_ROM)
	{
		build/api/?* ( .text* )
		build/programs/?* ( .text* )
	}
	prog_code_size = SIZEOF(prog_code);

	prog_data_ROM = prog_code_ROM + prog_code_size;
	prog_data PROG_DATA_RAM : AT (prog_data_ROM)
	{
		build/api/?* ( .rodata* .data* .bss* COMMON* )
		build/programs/?* ( .rodata* .data* .bss* COMMON* )
	}
	prog_data_size = SIZEOF(prog_data);

	kernel_end_addr = PROG_DATA_RAM + prog_data_size;

	/* For stack, two options are shown here */
	/* 1. treat system_stack as a variable in special section and
	      mark it as such in kernel/startup.c */
	.stack_segment STACK_ADDR : AT (prog_data_ROM + prog_data_size)
	{
		* ( .stack_section* )
	}

	/* 2. define system_stack here instead in kernel/startup.c */
	/* system_stack = STACK_ADDR; */

	/DISCARD/ : { *(* .*) }
}
