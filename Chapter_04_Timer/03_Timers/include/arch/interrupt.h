/*! Interrupt handling - 'arch' layer (only basic operations) */
#pragma once

/*! Initialize interrupt subsystem (in 'arch' layer) */
void arch_init_interrupts ();

/*! Register handler function for particular interrupt number */
void arch_register_interrupt_handler ( unsigned int inum, void *handler );

/*! Unregister handler function for particular interrupt number */
void arch_unregister_interrupt_handler ( unsigned int irq_num, void *handler );

/*!
 * enable and disable interrupts generated outside processor, controller by
 * interrupt controller (PIC or APIC or ...)
 */
void arch_irq_enable ( unsigned int irq );
void arch_irq_disable ( unsigned int irq );

#define INT_STF			12	/* Stack Fault */
#define INT_GPF			13	/* General Protection Fault */

#include <ARCH/interrupt.h>	/* for SOFT_IRQ */
#define SOFTWARE_INTERRUPT	SOFT_IRQ
