/*! Startup function - initialize kernel subsystem */
#define _K_STARTUP_C_

#include "memory.h"
#include "kprint.h"
#include <arch/interrupt.h>
#include <arch/processor.h>
#include <api/stdio.h>
#include <api/prog_info.h>
#include <types/io.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" ARCH ")";

/*!
 * First kernel function (after boot loader loads it to memory)
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

	/* interrupts */
	arch_init_interrupts ();

	/* detect memory faults (qemu do not detect segment violations!) */
	arch_register_interrupt_handler ( INT_MEM_FAULT, k_memory_fault );
	arch_register_interrupt_handler ( INT_UNDEF_FAULT, k_memory_fault );

	/* switch to default 'stdout' for kernel */
	k_stdout = &K_STDOUT;
	k_stdout->init (0);

	kprintf ( "%s\n", system_info );

	/* enable interrupts */
	enable_interrupts ();

	stdio_init (); /* initialize standard output devices */

	/* start desired program(s) */
	hello_world ();
	segm_fault ();

#if ( TURN_OFF == 0 )
	kprintf ( "\nSystem halted!\n" );
#ifdef PL190
	/* testing uart (arm) interrupts */
	void uart0_echo_test_start ();
	uart0_echo_test_start ();
	suspend();
#endif
	halt ();
#else
	/* power off (if supported, or just stop if not) */
	kprintf ( "Powering off\n\n" );
	power_off ();
#endif
}
