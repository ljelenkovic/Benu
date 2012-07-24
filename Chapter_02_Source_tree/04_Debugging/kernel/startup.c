/*! Startup function - initialize kernel subsystem */
#define _K_STARTUP_C_

#include "memory.h"
#include "kprint.h"
#include <arch/processor.h>
#include <api/stdio.h>
#include <api/prog_info.h>
#include <types/io.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" PLATFORM ")";

/*!
 * First kernel function (after grub loads it to memory)
 */
void k_startup ()
{
	extern console_t K_INITIAL_STDOUT, K_STDOUT;
	extern console_t *k_stdout; /* console for kernel messages */

	/* set initial stdout */
	k_stdout = &K_INITIAL_STDOUT;
	k_stdout->init (0);

	/* initialize memory subsystem (needed for boot) */
	k_memory_init ();

	/*! start with regular initialization */

	/* switch to default 'stdout' for kernel */
	k_stdout = &K_STDOUT;
	k_stdout->init (0);

	kprintf ( "%s\n", system_info );

	stdio_init (); /* initialize standard output devices */

	/* start desired program(s) */
	hello_world ();
	debug ();

	kprintf ( "\nSystem halted!\n" );
	halt ();
}
