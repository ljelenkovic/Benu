/*! Interrupt handling - 'arch' layer (only basic operations) */

#define _ARCH_INTERRUPTS_C_
#include "interrupt.h"

#include <arch/processor.h>
#include <kernel/errno.h>
#include "list.h"
#include <kernel/memory.h>

/*! Interrupt controller device */
extern arch_ic_t IC_DEV;
static arch_ic_t *icdev = &IC_DEV;

/*! interrupt handlers */
static struct list_head ihandlers[INTERRUPTS];

struct ihndlr
{
	int (*ihandler)(unsigned int);

	struct list_head list;
};

/*! Initialize interrupt susubsystem (in 'arch' layer) */
void arch_init_interrupts()
{
	int i;

	icdev->init();

	for (i = 0; i < INTERRUPTS; i++)
		INIT_LIST_HEAD(&ihandlers[i]);
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
void arch_register_interrupt_handler(unsigned int inum, void *handler)
{
	struct ihndlr *ih;

	if (inum < INTERRUPTS)
	{
		ih = kmalloc(sizeof(struct ihndlr));
		ASSERT(ih);

		ih->ihandler = handler;

		list_add_tail(&ih->list, &ihandlers[inum]);
	}
	else {
		LOG(ERROR, "Interrupt %d can't be used!\n", inum);
		halt();
	}
}

/*! Unregister handler function for particular interrupt number */
void arch_unregister_interrupt_handler(unsigned int irq_num, void *handler)
{
	struct list_head *pos, *n;

	ASSERT(irq_num >= 0 && irq_num < INTERRUPTS);

	list_for_each_safe(pos, n, &ihandlers[irq_num])
	{
		struct ihndlr *ih = list_entry(pos, struct ihndlr, list);
		if (ih->ihandler == handler)
			list_del(&ih->list);
	}
}

/*!
 * "Forward" interrupt handling to registered handler
 * (called from interrupts.S)
 */
void arch_interrupt_handler(int irq_num)
{
	if (irq_num < INTERRUPTS &&  !list_empty(&ihandlers[irq_num]))
	{
		/* enable interrupts on PIC immediately since program may not
		 * return here immediately */
		if (icdev->at_exit)
			icdev->at_exit(irq_num);

		/* Call registered handlers */
		struct ihndlr *ih;
		list_for_each_entry(ih, &ihandlers[irq_num], list)
			ih->ihandler(irq_num);
	}

	else if (irq_num < INTERRUPTS)
	{
		LOG(ERROR, "Unregistered interrupt: %d - %s!\n",
		      irq_num, icdev->int_descr(irq_num));
		halt();
	}
	else {
		LOG(ERROR, "Unregistered interrupt: %d !\n", irq_num);
		halt();
	}
}
