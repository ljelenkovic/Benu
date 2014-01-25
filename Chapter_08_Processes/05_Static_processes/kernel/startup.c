/*! Startup function - initialize kernel subsystem */
#define _K_STARTUP_C_

#include "time.h"
#include "thread.h"
#include "syscall.h"
#include "device.h"
#include "memory.h"
#include <kernel/errno.h>
#include <arch/interrupt.h>
#include <arch/processor.h>
#include <lib/string.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" ARCH ")";

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
	arch_register_interrupt_handler ( SOFTWARE_INTERRUPT, k_syscall, NULL );

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

	/* thread subsystem */
	kthreads_init ();

	/* start inital program, defined in Makefile - create process */
	if ( !kthread_start_process ( K_INIT_PROG, NULL, 0 ) )
	{
		LOG ( ERROR, "\nAborting!\n" );
		halt();
	}

	/* complete initialization by starting first thread */
	arch_return_to_thread ();
}
