/*! Interrupt handling - 'arch' layer (only basic operations) */
#pragma once

/*! Initialize interrupt susubsystem (in 'arch' layer) */
void arch_init_interrupts();

/*! Register handler function for particular interrupt number */
void *arch_register_interrupt_handler(unsigned int inum, void *handler);

/*! Unregister handler function for particular interrupt number */
void *arch_unregister_interrupt_handler(unsigned int irq_num);

#include <ARCH/interrupt.h>	/* for SOFT_IRQ */
#define SOFTWARE_INTERRUPT	SOFT_IRQ
