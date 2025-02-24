/*! Processor/thread context */

#pragma once

#include <types/basic.h>
#include <arch/context.h>
#include <arch/processor.h>

#define	INIT_CPSR	CPSR_MODE_SYS /* system mode, interrupts enabled ! */


/*! context manipulation ---------------------------- */

typedef struct _arch_context_t_
{
	uint32  cpsr;
	uint32  r[14]; /* {r0-r12,lr} */
	/* r0 - 1st time: for parameter to initial function */
	/* lr - 1st time: for thread exit function */
	uint32  pc;	/* 1st time: thread starting function */
}
arch_context_t;
/* __attribute__((__packed__)) not required since all elem. are 32 bits wide */

/*
 * NOTE on push/pop (arm processor)
 * The lowest numbered register is transferred to or from the lowest
 * memory address accessed, and the highest numbered register to or from the
 * highest address accessed. The order of the registers in the register list
 * in the instructions makes no difference.
 */

/*! Thread context */
struct _context_t_
{
	arch_context_t  *context;   /* thread context is on stack! */
};
