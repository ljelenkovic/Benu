/*! Startup function - initialize kernel subsystem */
#define _K_STARTUP_C_

#include "time.h"
#include "thread.h"
#include "device.h"
#include "memory.h"
#include "kprint.h"
#include <arch/interrupt.h>
#include <lib/string.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" PLATFORM ")";

/*!
 * First kernel function (after grub loads it to memory)
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
	arch_register_interrupt_handler ( INT_STF, k_memory_fault, NULL );
	arch_register_interrupt_handler ( INT_GPF, k_memory_fault, NULL );

	/* timer subsystem */
	k_time_init ();

	/* devices */
	k_devices_init ();

	/* switch to default 'stdout' for kernel */
	k_stdout = k_device_open ( K_STDOUT, O_WRONLY );

	kprintf ( "%s\n", system_info );

	if ( strcmp ( U_STDIN, "i8042" ) == 0 )
		kprintf ("For input (keyboard) focus QEMU simulator window!\n");
	else if ( strcmp ( U_STDIN, "COM1" ) == 0 )
		kprintf ("For input (keyboard) focus shell\n");

	/* thread subsystem */
	kthreads_init (); /* there is no return from it */
}
