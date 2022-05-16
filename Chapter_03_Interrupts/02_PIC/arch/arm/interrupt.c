/*! Interrupt handling - 'arch' layer (only basic operations) */

#define _ARCH_INTERRUPTS_C_
#include "interrupt.h"

#include <arch/processor.h>
#include <kernel/errno.h>
#include <lib/string.h>

/*! Interrupt controller device */
extern arch_ic_t IC_DEV;
static arch_ic_t *icdev = &IC_DEV;

/*! interrupt handlers */
static void (*ihandler[INTERRUPTS])(unsigned int);

/*! Initialize interrupt susubsystem (in 'arch' layer) */
void arch_init_interrupts()
{
	icdev->init();

	memset(ihandler, 0, sizeof(ihandler));
}

/*!
 * enable and disable interrupts generated outside processor, controller by
 * interrupt controller (PIC or APIC or ...)
 */
void arch_irq_enable(unsigned int irq)
{
	icdev->enable_irq(irq);
}
void arch_irq_disable(unsigned int irq)
{
	icdev->disable_irq(irq);
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

	/* retrieve handler number from interrupt controller for IRQ and FIQ */
	if (irqn == INT_SRC_IRQ || irqn == INT_SRC_FIQ)
		irqn = icdev->get_irq();

	if (irqn > 0 && irqn < INTERRUPTS)
	{
		if (ihandler[irqn])
		{
			/* Call registered handler */
			ihandler[irqn](irqn);
		}
		else {
			LOG(ERROR, "Unregistered interrupt: %d!\n(%s)\n",
			      irqn, icdev->int_descr(irqn));
			halt();
		}
	}
	else {
		LOG(ERROR, "Unknown interrupt: %d !\n", irqn);
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
