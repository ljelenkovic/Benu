/*! Startup function - initialize kernel subsystem */
#define _K_STARTUP_C_

#include <kernel/kprint.h>
#include <arch/processor.h>
#include <api/prog_info.h>

/*! kernel stack */
uint8 system_stack [ STACK_SIZE ];

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" ARCH ")";

/*!
 * First kernel function (after boot loader loads it to memory)
 */
void k_startup()
{
	/* initialize "subsystem" */
	kconsole_init();

	kconsole_print_word(system_info);

	/* start desired program(s) */
	hello_world();

	kconsole_print_word("System halted!");
	halt();
}
