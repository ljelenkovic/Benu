/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include "context.h"

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
	context->context->r[0] = (uint32) param;
	context->context->r[13] = (uint32) thread_exit;
	context->context->cpsr = INIT_CPSR;

	/* rest of context->context is not relevant for new thread */
}
