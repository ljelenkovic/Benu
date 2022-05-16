/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include "context.h"

#include "interrupt.h"
#include "descriptor.h"
#include <kernel/memory.h>

/*! context manipulation ---------------------------------------------------- */

/*! Create initial context for thread - it should start with defined function
 */
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

	/* thread context */
	context->context->eip = (uint32) func;
	context->context->eflags = INIT_EFLAGS;

	/* rest of context->context is not relevant for new thread */
}

/* switch from one thread to another */
void arch_switch_to_thread(context_t *from, context_t *to)
{
	asm volatile (
		"cmpl	$0, %1		\n\t"	/* is "from" given? */
		"je	1f		\n\t"

		"pushl	$2f		\n\t"	/* EIP */
		"pushfl			\n\t"	/* EFLAGS */
		"pushal			\n\t"	/* all registers */
		"movl	%%esp, %0	\n\t"	/* save stack => from */

	"1:	 movl	%2, %%esp	\n\t"	/* restore stack <= to*/
		"popal			\n\t"
		"popfl			\n\t"
		"ret			\n\t"

	"2:	 nop			\n\t"

		: "=m" (from->context)	/* %0 */
		: "m" (from), 		/* %1 */
		  "m" (to->context)	/* %2 */
	);
}

/* switch stack and call thread_exit2 */
void arch_thread_exit_with_stack_switch(void *kthread, void *exit_status)
{
	extern uint8 system_stack[];

	asm volatile (
	/* save parameters from stack to registers */
	"movl	%0, %%eax	\n\t"
	"movl	%1, %%ebx	\n\t"

	"movl	%2, %%esp	\n\t"	/* switch to bootup stack */

	/* push parameters from registers to stack */
	"pushl	%%ebx		\n\t"
	"pushl	%%eax		\n\t"

	"call	kthread_exit2	\n\t"
	/* there is no return to here */

		:: "m" (kthread), 				/* %0 */
		   "m" (exit_status),				/* %1 */
		   "g" (system_stack + KERNEL_STACK_SIZE)	/* %3 */
		: "%eax", "%ebx", "%ecx"
	);
}
