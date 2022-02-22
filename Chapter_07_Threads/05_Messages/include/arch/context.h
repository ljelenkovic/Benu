/*! Processor/thread context */
#pragma once

void arch_descriptors_init();

/*!
 * Context structures and operations: context_t
 */

struct _context_t_; typedef struct _context_t_ context_t;

/*! Create initial context for thread - it should start with defined function
 */
void arch_create_thread_context(
	context_t *context,
	void (func)(void *), void *param,
	void (*thread_exit)(),
	void *stack, size_t stack_size
);

void arch_switch_to_thread(context_t *from, context_t *to);
void arch_thread_exit_with_stack_switch(void *kthread, void *exit_status);

#include <ARCH/context.h> /* for context_t */
