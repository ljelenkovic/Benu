/*! Intel 8259 programmable interrupt controller (PIC) */
#ifdef I8259

#pragma once

#define	IRQ_OFFSET	0x20	/* Interrupt offset for external interrupts */

#ifndef ASM_FILE

/*! External interrupts */
enum {
	IRQ_TIMER = IRQ_OFFSET,
	IRQ_KEYBOARD,
	IRQ_SLAVE_PIC,
	IRQ_COM2,
	IRQ_COM1,
	IRQ_LPT2,
	IRQ_FLOPPY,
	IRQ_LPT1,
	IRQ_RT_CLOCK,
	IRQ_MASTER_PIC,
	IRQ_RESERVED1,
	IRQ_RESERVED2,
	IRQ_RESERVED3,
	IRQ_COPROCESSOR,
	IRQ_HARD_DISK,
	IRQ_RESERVED4,

	HW_INTERRUPTS
};

#define SOFT_IRQ	HW_INTERRUPTS

#else /* ASM_FILE */

#define SOFT_IRQ	48 /* NOTE: adjust to match 'HW_INTERRUPTS' !!! */

#endif /* ASM_FILE */

#define NUM_IRQS	(SOFT_IRQ + 1)

#endif /* I8259 */
