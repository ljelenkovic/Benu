/*! Interrupt handling - 'arch' layer (only basic operations) */

#pragma once

/* Constants */

/*! Interrupt sources */
#define INT_SRC_UNUSED		0
#define INT_SRC_UNDEF		1
#define INT_SRC_SWI		2
#define INT_SRC_PRE_ABORT	3
#define INT_SRC_ABORT		4
#define INT_SRC_IRQ		5
#define INT_SRC_FIQ		6
#define INT_SRC_NUM		7

#define SOFT_IRQ		INT_SRC_SWI
#define INT_MEM_FAULT		INT_SRC_ABORT
#define INT_UNDEF_FAULT		INT_SRC_UNDEF

/*! IRQ interrupts: 0-31 from primary controller, 0-31 from secondary */
#define INTERRUPTS	(INT_SRC_NUM + 32 + 32)
#define IRQ_OFFSET	INT_SRC_NUM

#ifndef ASM_FILE

#include <arch/interrupt.h>

/*! (Hardware) Interrupt controller interface */
typedef struct _interrupt_controller_
{
	void  (*init)();
	void  (*disable_irq)(unsigned int irq);
	void  (*enable_irq)(unsigned int irq);
	void  (*at_exit)(unsigned int irq);
	int   (*get_irq)();

	char  *(*int_descr)(unsigned int irq);
}
arch_ic_t;

/* PrimeCell Vectored Interrupt Controller (PL190) */
#include <ARCH/drivers/pl190.h>

#endif /* ASM_FILE */
