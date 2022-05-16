/*! Processor/thread context */

#pragma once

#include <types/basic.h>
#include <arch/context.h>
#include <arch/processor.h>

#define	INIT_CPSR(CPSR_MODE_SVC | CPSR_IRQ) /* SVC, interrupts disabled ! */
#define	INIT_SPSR CPSR_MODE_USR /* user mode, interrupts enabled ! */


/*! context manipulation ---------------------------- */

typedef struct _arch_context_t_
{
	uint32  cpsr;	/* interrupt mode */
	uint32  spsr;	/* thread mode (system) */
	uint32  pc;	/* 1st time: thread starting function */
	uint32  rh[11];	/* {r4-r14} */
	uint32  rl[4];	/* {r0-r3} (parameters to syscall) */
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


/*! context manipulation - for 'user threads' (in programs) ----------------- */
#define	INIT_CPSR_U	CPSR_MODE_SYS /* system mode, interrupts enabled ! */

typedef struct _arch_ucontext_t_
{
	uint32  cpsr;
	uint32  r[14]; /* {r0-r12,lr} */
	/* r0 - 1st time: for parameter to initial function */
	/* lr - 1st time: for thread exit function */
	uint32  pc;	/* 1st time: thread starting function */
}
arch_ucontext_t;
/* __attribute__((__packed__)) not required since all elem. are 32 bits wide */

/*! Thread context */
struct _ucontext_t_
{
	arch_ucontext_t  *context;   /* thread context is on stack! */
};

/*! Create initial context for thread - it should start with defined function
 */
static inline void arch_create_uthread_context(ucontext_t *context,
		void (func)(void *), void *param, void (thread_exit)(),
		void *stack, size_t stack_size)
{
	/* thread context is on stack */
	context->context = (void *)
		((uint32) stack) + stack_size - sizeof(arch_ucontext_t);

	/* thread context */
	context->context->pc = (uint32) func;
	context->context->r[0] = (uint32) param;
	context->context->r[13] = (uint32) thread_exit;
	context->context->cpsr = INIT_CPSR_U;

	/* rest of context->context is not relevant for new thread */
}

static inline void arch_switch_to_uthread(ucontext_t *from, ucontext_t *to)
{
	void arch_switch_to_uthread2(ucontext_t *from, ucontext_t *to);

	arch_switch_to_uthread2(from, to);
}
