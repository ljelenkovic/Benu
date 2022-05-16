/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include "context.h"

#include "interrupt.h"
#include "descriptor.h"
#include <kernel/memory.h>

/*! kernel (interrupt) stack (defined in memory.c) */
extern uint8 system_stack [];

/*! interrupt handler stack */
void *arch_interrupt_stack;

/*! where is thread context saved at interrupt? */
context_t *arch_active_thr_context;
uint32 *arch_thr_context;
#ifdef USE_SSE
uint32 arch_sse_supported = 0; /* is SSE supported by processor? */
uint32 arch_sse_mmx_fpu;	/* where to save extended thread context */
#endif

/*! Set up context (normal and interrupt=kernel) */
void arch_context_init()
{
	arch_interrupt_stack = (void *) &system_stack [ KERNEL_STACK_SIZE ];

	arch_descriptors_init(); /* GDT, IDT, ... */
}

/*! context manipulation ---------------------------------------------------- */

/*! Create initial context for thread - it should start with defined function
  (context is identical to interrupt frame - use same code to start/return) */
void arch_create_thread_context(context_t *context,
		void (func)(void *), void *param, void (*thread_exit)(),
		void *stack, size_t stack_size)
{
	uint32 *tstack;

	/* thread stack */
	tstack = stack + stack_size;

	/* put starting thread function parameter on stack */
	*(--tstack) = (uint32) param;
	/* return address (when thread exits */
	*(--tstack) = (uint32) thread_exit;

	/* thread context is on stack */
	context->context = (void *) tstack - sizeof(arch_context_t);

	/* interrupt frame */
	context->context->eflags = INIT_EFLAGS;
	context->context->cs = GDT_DESCRIPTOR(SEGM_CODE, GDT, PRIV_KERNEL);
	context->context->eip = (uint32) func;

	/* rest of context->context is not relevant for new thread */

#ifdef USE_SSE
	if (arch_sse_supported)
	{
		uint32 top = (uint32) context->context;

		/* align on 16 B address */
		context->sse_mmx_fpu = (top - 512) & 0xfffffff0;

		/* init SSE context - use current for initial */
		asm volatile (
			"mov	%0, %%eax	\n\t"
			"fxsave	(%%eax)		\n\t"
			:: "m" (context->sse_mmx_fpu)
			: "%eax"
		);
	}
#endif
}

/*! Select thread to return to from interrupt */
void arch_select_thread(context_t *context)
{
	arch_active_thr_context = context;
	arch_thr_context = (void *) context->context;

#ifdef USE_SSE
	arch_sse_mmx_fpu = context->sse_mmx_fpu;
#endif
}
