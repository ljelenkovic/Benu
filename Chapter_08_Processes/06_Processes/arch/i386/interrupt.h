/*! Interrupt handling - 'arch' layer (only basic operations) */

#pragma once

#ifndef ASM_FILE

#include <arch/interrupt.h>

/*! (Hardware) Interrupt controller interface */
typedef struct _interrupt_controller_
{
	void   (*init) ();
	void   (*disable_irq) ( unsigned int irq );
	void   (*enable_irq) ( unsigned int irq );
	void   (*at_exit) ( unsigned int irq );

	char  *(*int_descr) ( unsigned int irq );
}
arch_ic_t;

#endif /* ASM_FILE */

/* Programmable Interrupt controllers (currently implemented only one, i8259) */
#include <ARCH/drivers/i8259.h>

/* Constants */
#define INTERRUPTS		NUM_IRQS
