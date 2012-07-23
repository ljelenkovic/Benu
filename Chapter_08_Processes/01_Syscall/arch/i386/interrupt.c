/*! Interrupt handling - 'arch' layer (only basic operations) */

#define _ARCH_INTERRUPTS_C_
#include "interrupt.h"

#include "io.h"
#include <arch/processor.h>
#include <arch/context.h>
#include <kernel/errno.h>
#include <lib/list.h>
#include <kernel/memory.h>

/*! Interrupt controller device */
extern arch_ic_t IC_DEV;
static arch_ic_t *icdev = &IC_DEV;

void (*arch_irq_enable_func) ( unsigned int );
void (*arch_irq_disable_func) ( unsigned int );

/*! interrupt handlers */
static list_t ihandlers[INTERRUPTS];

/*!
 * interrupted: user program or kernel
 * (for tracking processor generated interrupts)
 */
static int new_mode = KERNEL_MODE;
static int prev_mode = KERNEL_MODE;

struct ihndlr
{
	void *device;
	int (*ihandler) ( unsigned int, void *device );

	list_h list;
};

/*! Initialize interrupt subsystem (in 'arch' layer) */
void arch_init_interrupts ()
{
	int i;

	icdev->init ();

	arch_irq_enable_func = icdev->enable_irq;
	arch_irq_disable_func = icdev->disable_irq;

	for ( i = 0; i < INTERRUPTS; i++ )
		list_init ( &ihandlers[i] );
}

/*!
 * enable and disable interrupts generated outside processor, controller by
 * interrupt controller (PIC or APIC or ...)
 */
void arch_irq_enable ( unsigned int irq )
{
	arch_irq_enable_func ( irq );
}
void arch_irq_disable ( unsigned int irq )
{
	arch_irq_disable_func ( irq );
}

/*! Register handler function for particular interrupt number */
void arch_register_interrupt_handler ( unsigned int inum, void *handler,
				       void *device )
{
	struct ihndlr *ih;

	if ( inum < INTERRUPTS )
	{
		ih = kmalloc ( sizeof (struct ihndlr) );
		ASSERT ( ih );

		ih->device = device;
		ih->ihandler = handler;

		list_append ( &ihandlers[inum], ih, &ih->list );
	}
	else {
		LOG ( ERROR, "Interrupt %d can't be used!\n", inum );
		halt ();
	}
}

/*! Unregister handler function for particular interrupt number */
void arch_unregister_interrupt_handler ( unsigned int irq_num, void *handler,
					 void *device )
{
	struct ihndlr *ih, *next;

	ASSERT ( irq_num >= 0 && irq_num < INTERRUPTS );

	ih = list_get ( &ihandlers[irq_num], FIRST );

	while ( ih )
	{
		next = list_get_next ( &ih->list );

		if ( ih->ihandler == handler && ih->device == device )
			list_remove ( &ihandlers[irq_num], FIRST, &ih->list );

		ih = next;
	}
}

/*!
 * "Forward" interrupt handling to registered handler
 * (called from interrupts.S)
 */
void arch_interrupt_handler ( int irq_num )
{
	struct ihndlr *ih;
	extern uint32 *arch_thr_context;
	extern context_t *arch_active_thr_context;
#ifdef USE_SSE
	extern uint32 arch_sse_mmx_fpu;
	arch_active_thr_context->sse_mmx_fpu = arch_sse_mmx_fpu;
#endif
	arch_active_thr_context->context = (void *) arch_thr_context;

	prev_mode = new_mode;
	new_mode = KERNEL_MODE;

	if ( irq_num < INTERRUPTS && (ih = list_get (&ihandlers[irq_num], FIRST)) )
	{
		/* Call registered handlers */
		while ( ih )
		{
			ih->ihandler ( irq_num, ih->device );

			ih = list_get_next ( &ih->list );
		}

		if ( icdev->at_exit )
			icdev->at_exit ( irq_num );
	}

	else if ( irq_num < INTERRUPTS )
	{
		LOG ( ERROR, "Unregistered interrupt: %d - %s!\n",
		      irq_num, icdev->int_descr ( irq_num ) );
		halt ();
	}
	else {
		LOG ( ERROR, "Unregistered interrupt: %d !\n", irq_num );
		halt ();
	}

	prev_mode = new_mode;
	new_mode = USER_MODE;
}

/*! return current processor operating mode (KERNEL_MODE or USER_MODE) */
int arch_new_mode ()
{
	return new_mode;
}

/*! return previous processor operating mode (KERNEL_MODE or USER_MODE) */
int arch_prev_mode ()
{
	return prev_mode;
}