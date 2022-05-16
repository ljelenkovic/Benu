/*! Interrupt handling - 'arch' layer (only basic operations) */
#pragma once

/*! Initialize interrupt susubsystem (in 'arch' layer) */
void arch_init_interrupts();

/*! Register handler function for particular interrupt number */
void arch_register_interrupt_handler(
	unsigned int inum,
	void *handler,
	void *device
);

/*! Unregister handler function for particular interrupt number */
void arch_unregister_interrupt_handler(
	unsigned int irq_num,
	void *handler,
	void *device
);

/*! Quit startup thread and start with created one (=> arch_select_thread) */
void arch_return_to_thread();

/*!
 * enable and disable interrupts generated outside processor, controller by
 * interrupt controller (PIC or APIC or ...)
 */
void arch_irq_enable(unsigned int irq);
void arch_irq_disable(unsigned int irq);

/*! detecting segmentation faults - from threads or kernel */

/*! return current processor operating mode (KERNEL_MODE or USER_MODE) */
int arch_new_mode();

/*! return previous processor operating mode (KERNEL_MODE or USER_MODE) */
int arch_prev_mode();

#define KERNEL_MODE		0
#define USER_MODE		-1

#include <ARCH/interrupt.h>	/* for SOFT_IRQ */
#define SOFTWARE_INTERRUPT	SOFT_IRQ
