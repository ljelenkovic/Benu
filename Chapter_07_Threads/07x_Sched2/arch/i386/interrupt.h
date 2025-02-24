/*! Interrupt handling - 'arch' layer (only basic operations) */

#pragma once

/* Constants */
#define INT_STF			12	/* Stack Fault */
#define INT_GPF			13	/* General Protection Fault */

#define INT_MEM_FAULT		INT_STF
#define INT_UNDEF_FAULT		INT_GPF

#ifndef ASM_FILE

#include <arch/interrupt.h>

/*! (Hardware) Interrupt controller interface */
typedef struct _interrupt_controller_
{
	void  (*init)();
	void  (*disable_irq)(unsigned int irq);
	void  (*enable_irq)(unsigned int irq);
	void  (*at_exit)(unsigned int irq);

	char  *(*int_descr)(unsigned int irq);
}
arch_ic_t;

#endif /* ASM_FILE */

/* Programmable Interrupt controllers (currently implemented only one, i8259) */
#include <ARCH/drivers/i8259.h>

#define INTERRUPTS		(SOFT_IRQ + 1)
