/*! Processor/thread context */
#pragma once

void arch_descriptors_init ();

/*!
 * Context structures and operations: context_t
 */

struct _context_t_; typedef struct _context_t_ context_t;

/*! Create initial context for thread - it should start with defined function
   (context is identical to interrupt frame - use same code to start/return) */
void arch_create_thread_context (
	context_t *context,
	void (func) (void *), void *param,
	void (*thread_exit)(),
	void *stack, size_t stack_size
);

void arch_switch_to_thread ( context_t *from, context_t *to );
void arch_switch_to_thread_with_cleanup ( void *from, context_t *to );

struct _ucontext_t_; typedef struct _ucontext_t_ ucontext_t;

#include <ARCH/context.h> /* for context_t and ucontext_t */
