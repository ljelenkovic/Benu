/*! Processor/thread context */
#pragma once

#include <types/basic.h>
/*!
 * Context structures and operations: context_t
 */

struct _context_t_; typedef struct _context_t_ context_t;

/*! Set up context (normal and interrupt=kernel) */
void arch_context_init();

/*! Create initial context for thread - it should start with defined function
  (context is identical to interrupt frame - use same code to start/return) */
void arch_create_thread_context(
	context_t *context,
	void (func)(void *), void *param,
	void (*thread_exit)(),
	void *stack, size_t stack_size
);

/*! Select thread to return to from interrupt (from syscall) */
void arch_select_thread(context_t *cntx);

/*!
 * For 'user threads' (in programs) (use inline, they are included from program)
 */

struct _ucontext_t_; typedef struct _ucontext_t_ ucontext_t;

static inline void arch_create_uthread_context(
	ucontext_t *context,
	void (func)(void *), void *param,
	void (thread_exit)(),
	void *stack, size_t stack_size
);

static inline void arch_switch_to_uthread(ucontext_t *from, ucontext_t *to);

#include <ARCH/context.h> /* for context_t and ucontext_t */
