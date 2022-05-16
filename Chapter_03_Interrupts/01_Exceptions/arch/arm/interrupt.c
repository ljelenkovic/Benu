/*! Interrupt handling - 'arch' layer (only basic operations) */

#define _ARCH_INTERRUPTS_C_
#include "interrupt.h"

#include <arch/processor.h>
#include <kernel/errno.h>
#include <lib/string.h>

/*! interrupt handlers */
static void (*ihandler[INTERRUPTS])(unsigned int);

/*! Initialize interrupt susubsystem (in 'arch' layer) */
void arch_init_interrupts()
{
	memset(ihandler, 0, sizeof(ihandler));
}

/*! Register handler function for particular interrupt number */
void *arch_register_interrupt_handler(unsigned int irqn, void *handler)
{
	void *old_handler = NULL;

	if (irqn < INTERRUPTS)
	{
		old_handler = ihandler[irqn];
		ihandler[irqn] = handler;
	}
	else {
		LOG(ERROR, "Interrupt %d can't be used!\n", irqn);
		halt();
	}

	return old_handler;
}

/*! Unregister handler function for particular interrupt number */
void *arch_unregister_interrupt_handler(unsigned int irqn)
{
	void *old_handler = NULL;
	ASSERT(irqn >= 0 && irqn < INTERRUPTS);

	old_handler = ihandler[irqn];
	ihandler[irqn] = NULL;

	return old_handler;
}

/*!
 * "Forward" interrupt handling to registered handler
 * (called from interrupts.S)
 */
void arch_interrupt_handler(int cpsr)
{
	int irqn = arch_get_irqn(cpsr & 0x001f);

	if (irqn > 0 && irqn < INTERRUPTS && ihandler[irqn])
	{
		/* Call registered handler */
		ihandler[irqn](irqn);
	}
	else {
		LOG(ERROR, "Unknown or unregistered interrupt: %d !\n", irqn);
		halt();
	}
}

/*
 * Interrupt handlers written in C (not used; defined in interrupt.S)
 * - all interrupt handler functions redirect to same handler
 */
/*void __attribute__((interrupt("UNDEF"))) arch_undef_hndl()
{
	arch_interrupt_handler(INT_SRC_UNDEF);
}
void __attribute__((interrupt("SWI"))) arch_swi_hndl()
{
	arch_interrupt_handler(INT_SRC_SWI);
}
void __attribute__((interrupt("ABORT"))) arch_prefetch_abort_hndl()
{
	arch_interrupt_handler(INT_SRC_PRE_ABORT);
}
void __attribute__((interrupt("ABORT"))) arch_data_abort_hndl()
{
	arch_interrupt_handler(INT_SRC_ABORT);
}
void __attribute__((interrupt("IRQ"))) arch_irq_hndl()
{
	arch_interrupt_handler(INT_SRC_IRQ);
}
void __attribute__((interrupt("FIQ"))) arch_fiq_hndl()
{
	arch_interrupt_handler(INT_SRC_FIQ);
}*/
