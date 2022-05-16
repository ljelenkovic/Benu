/*! Interrupt handling - 'arch' layer (only basic operations) */

#define _ARCH_INTERRUPTS_C_
#include "interrupt.h"

#include <arch/processor.h>
#include <kernel/errno.h>
#include <lib/list.h>
#include <kernel/memory.h>

/*! Interrupt controller device */
extern arch_ic_t IC_DEV;
static arch_ic_t *icdev = &IC_DEV;

/*! interrupt handlers */
static list_t ihandlers[INTERRUPTS];

struct ihndlr
{
	void *device;
	int (*ihandler)(unsigned int, void *device);

	list_h list;
};

/*! Initialize interrupt susubsystem (in 'arch' layer) */
void arch_init_interrupts()
{
	int i;

	icdev->init();

	for (i = 0; i < INTERRUPTS; i++)
		list_init(&ihandlers[i]);
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
void arch_register_interrupt_handler(unsigned int inum, void *handler,
				       void *device)
{
	struct ihndlr *ih;

	if (inum < INTERRUPTS)
	{
		ih = kmalloc(sizeof(struct ihndlr));
		ASSERT(ih);

		ih->device = device;
		ih->ihandler = handler;

		list_append(&ihandlers[inum], ih, &ih->list);
	}
	else {
		LOG(ERROR, "Interrupt %d can't be used!\n", inum);
		halt();
	}
}

/*! Unregister handler function for particular interrupt number */
void arch_unregister_interrupt_handler(unsigned int irq_num, void *handler,
					 void *device)
{
	struct ihndlr *ih, *next;

	ASSERT(irq_num >= 0 && irq_num < INTERRUPTS);

	ih = list_get(&ihandlers[irq_num], FIRST);

	while (ih)
	{
		next = list_get_next(&ih->list);

		if (ih->ihandler == handler && ih->device == device)
			list_remove(&ihandlers[irq_num], FIRST, &ih->list);

		ih = next;
	}
}

/*!
 * "Forward" interrupt handling to registered handler
 * (called from interrupts.S)
 */
void arch_interrupt_handler(int cpsr)
{
	struct ihndlr *ih;
	int irqn = arch_get_irqn(cpsr & 0x001f);

	/* retrieve handler number from interrupt controller for IRQ and FIQ */
	if (irqn == INT_SRC_IRQ || irqn == INT_SRC_FIQ)
		irqn = icdev->get_irq();

	if (irqn > 0 && irqn < INTERRUPTS)
	{
		if ((ih = list_get(&ihandlers[irqn], FIRST)) != NULL)
		{
			/* Call registered handlers */
			while (ih)
			{
				ih->ihandler(irqn, ih->device);

				ih = list_get_next(&ih->list);
			}
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
