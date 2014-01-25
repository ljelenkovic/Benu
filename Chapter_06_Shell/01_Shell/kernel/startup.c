/*! Startup function - initialize kernel subsystem */
#define _K_STARTUP_C_

#include "time.h"
#include "device.h"
#include "memory.h"
#include "kprint.h"
#include <arch/interrupt.h>
#include <arch/processor.h>
#include <api/stdio.h>
#include <api/prog_info.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" ARCH ")";

static void run_all ( char *argv[] );

/*!
 * First kernel function (after boot loader loads it to memory)
 */
void k_startup ()
{
	extern void *k_stdout; /* console for kernel messages */

	/* set initial stdout */
	kdevice_set_initial_stdout ();

	/* initialize memory subsystem (needed for boot) */
	k_memory_init ();

	/*! start with regular initialization */

	/* interrupts */
	arch_init_interrupts ();

	/* detect memory faults (qemu do not detect segment violations!) */
	arch_register_interrupt_handler ( INT_MEM_FAULT, k_memory_fault, NULL );
	arch_register_interrupt_handler ( INT_UNDEF_FAULT, k_memory_fault, NULL );

	/* timer subsystem */
	k_time_init ();

	/* devices */
	k_devices_init ();

	/* switch to default 'stdout' for kernel */
	k_stdout = k_device_open ( K_STDOUT, O_WRONLY );

	kprintf ( "%s\n", system_info );

	/* enable interrupts */
	enable_interrupts ();

	stdio_init (); /* initialize standard input & output devices */

	/* starting program routine */
	PROG_START_FUNC ( NULL );

#if ( TURN_OFF == 0 )
	kprintf ( "\nSystem halted!\n" );
	halt ();
#else
	/* power off (if supported, or just stop if not) */
	kprintf ( "Powering off\n\n" );
	power_off ();
#endif
}

static void run_all ( char *argv[] )
{
	kprintf ( "\nStarting program: hello_world\n\n" );
	hello_world (argv);

	kprintf ( "\nStarting program: timer\n\n" );
	timer (argv);
#if 0
	kprintf ( "\nStarting program: keyboard\n\n" );
	keyboard (argv);

	kprintf ( "\nStarting program: segm_fault\n\n" );
	segm_fault (argv);
#endif
}
