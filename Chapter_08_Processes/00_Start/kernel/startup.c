/*! Startup function - initialize kernel subsystem */
#define _K_STARTUP_C_

#include "time.h"
#include "thread.h"
#include "device.h"
#include "memory.h"
#include <kernel/errno.h>
#include <kernel/features.h>
#include <arch/interrupt.h>
#include <lib/string.h>

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" ARCH ")";

/* state of kernel features */
uint kernel_features = FEATURE_SUPPORTED; /* initially set all to "on" state */

/*!
 * First kernel function (after boot loader loads it to memory)
 */
void k_startup()
{
	extern void *k_stdout; /* console for kernel messages */

	/* set initial stdout */
	kdevice_set_initial_stdout();

	/* initialize memory subsystem (needed for boot) */
	k_memory_init();

	/*! start with regular initialization */

	/* interrupts */
	arch_init_interrupts();

	/* detect memory faults (qemu do not detect segment violations!) */
	arch_register_interrupt_handler(INT_MEM_FAULT, k_memory_fault, NULL);
	arch_register_interrupt_handler(INT_UNDEF_FAULT, k_memory_fault, NULL);

	/* timer subsystem */
	k_time_init();

	/* devices */
	k_devices_init();

	/* switch to default 'stdout' for kernel */
	k_stdout = k_device_open(K_STDOUT, O_WRONLY);

	kprintf("%s\n", system_info);

	/* thread subsystem */
	kthreads_init(); /* there is no return from it */
}

/*! Turn kernel feature on/off */
uint sys__feature(uint features, int cmd, int enable)
{
	uint prev_state = kernel_features & features;

	ASSERT(!(features & ~FEATURE_SUPPORTED));

	if (cmd == FEATURE_GET)
		return prev_state;

	/* update state */
	if (enable)
		kernel_features |= features;
	else
		kernel_features &= ~features;

	/* action required? */

	if ((features & FEATURE_INTERRUPTS))
	{
		if (enable)
			enable_interrupts();
		else
			disable_interrupts();
	}
#ifdef SCHED_RR_SIMPLE
	if ((features & FEATURE_SCHED_RR))
	{
		if (enable)
			ksched_rr_start_timer();
		else
			ksched_rr_stop_timer();
	}
#endif /* SCHED_RR_SIMPLE */

	return prev_state;
}
