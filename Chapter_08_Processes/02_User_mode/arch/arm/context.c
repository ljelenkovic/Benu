/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include "context.h"

/*! where is thread context saved at interrupt? */
volatile context_t *arch_active_thr_context;
volatile uint32 *arch_thr_context;


/*! context manipulation ---------------------------------------------------- */

/*! Create initial context for thread - it should start with defined function
 */
void arch_create_thread_context(context_t *context,
		void (func)(void *), void *param, void (*thread_exit)(),
		void *stack, size_t stack_size)
{
	/* thread context is on stack */
	context->context = (void *)
		((uint32) stack) + stack_size - sizeof(arch_context_t);

	/* thread context */
	context->context->pc = (uint32) func;
	context->context->rl[0] = (uint32) param;
	context->context->rh[9] = (uint32) stack + stack_size;	/* sp */
	context->context->rh[10] = (uint32) thread_exit;	/* lr */
	context->context->cpsr = INIT_CPSR;
	context->context->spsr = INIT_SPSR;

	/* rest of context->context is not relevant for new thread */
}

/*! Select thread to return to from interrupt */
void arch_select_thread(context_t *context)
{
	arch_active_thr_context = context;
	arch_thr_context = (void *) context;
	//arch_thr_context = (void *) &context->context;
}

/*! Cleanups on context when deleting thread (nothing to do) */
void arch_destroy_thread_context(context_t *context)
{
}
