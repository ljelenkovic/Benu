/*! Startup function - initialize kernel subsystem */
#define _K_STARTUP_C_

#include <arch/interrupt.h>
#include <arch/processor.h>
#include <api/stdio.h>
#include <api/prog_info.h>
#include <types/io.h>
#include <kernel/errno.h>
#include <kernel/features.h>

/*! kernel stack */
uint8 system_stack [ STACK_SIZE ];

char system_info[] = 	OS_NAME ": " NAME_MAJOR ":" NAME_MINOR ", "
			"Version: " VERSION " (" ARCH ")";

/* state of kernel features */
uint kernel_features = FEATURE_SUPPORTED; /* initially set all to "on" state */

static void k_memory_fault();

/*!
 * First kernel function (after boot loader loads it to memory)
 */
void k_startup()
{
	extern console_t K_INITIAL_STDOUT, K_STDOUT;
	extern console_t *k_stdout; /* console for kernel messages */

	/* set initial stdout */
	k_stdout = &K_INITIAL_STDOUT;
	k_stdout->init(0);

	/*! start with regular initialization */

	/* interrupts */
	arch_init_interrupts();

	/* detect memory faults (qemu do not detect segment violations!) */
	arch_register_interrupt_handler(INT_MEM_FAULT, k_memory_fault);
	arch_register_interrupt_handler(INT_UNDEF_FAULT, k_memory_fault);

	/* switch to default 'stdout' for kernel */
	k_stdout = &K_STDOUT;
	k_stdout->init(0);

	kprintf("%s\n", system_info);

	stdio_init(); /* initialize standard output devices */

	/* start desired program(s) */
	hello_world();
	segm_fault();

	kprintf("\nSystem halted!\n");
	halt();
}

/*! Handle memory fault interrupt(and others undefined) */
static void k_memory_fault()
{
	LOG(ERROR, "Undefined fault(exception)!!!");
	halt();
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

	return prev_state;
}
