/*! Assembler macros for some processor control instructions */

#pragma once

#define CPSR_IRQ	0xc0	/* I & F bits of CSPR */

#define CPSR_MODE_FIQ	0x11
#define CPSR_MODE_IRQ	0x12
#define CPSR_MODE_SVC	0x13
#define CPSR_MODE_ABT	0x17
#define CPSR_MODE_UND	0x1b
#define CPSR_MODE_SYS	0x1f

#define CPSR_MODE_SYS_IF	(CPSR_MODE_SYS|CPSR_IRQ)

#ifndef ASM_FILE

#define arch_disable_interrupts()	\
asm (	"push {r0}\n\t"			\
	"mrs r0, cpsr\n\t"		\
	"orr r0, r0, #0xc0\n\t"		\
	"msr cpsr, r0\n\t"		\
	"pop {r0}\n\t")

#define arch_enable_interrupts()	\
asm (	"push {r0}\n\t"			\
	"mrs r0, cpsr\n\t"		\
	"bic r0, r0, #0xc0\n\t"		\
	"msr cpsr, r0\n\t"		\
	"pop {r0}\n\t")

#define arch_halt()			\
do {					\
	arch_disable_interrupts();	\
	asm volatile ("b .\n\t");	\
} while (0)

#define arch_suspend()			asm ("" : : : "memory") /* not supp. */

#define arch_raise_interrupt(p)		asm volatile (	"svc %0\n\t" ::	\
							"i" (p):"memory")

#define arch_memory_barrier()		asm ("" : : : "memory")

#include "interrupt.h"

static inline int arch_get_irqn(int mode)
{
	switch(mode)
	{
	case CPSR_MODE_FIQ: return INT_SRC_FIQ;
	case CPSR_MODE_IRQ: return INT_SRC_IRQ;
	case CPSR_MODE_ABT: return INT_SRC_ABORT;
	case CPSR_MODE_SVC: return INT_SRC_SWI;
	case CPSR_MODE_UND: return INT_SRC_UNDEF;
	default: return -1;
	}
}

#endif /* ASM_FILE */
