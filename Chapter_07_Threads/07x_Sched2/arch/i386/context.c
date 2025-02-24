/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include "context.h"

#include "interrupt.h"
#include "descriptor.h"
#include <kernel/memory.h>

#ifdef USE_SSE
uint32 arch_sse_supported = 0;	/* is SSE supported by processor? */
#endif

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

/* switch from one thread to another */
void arch_switch_to_thread(context_t *from, context_t *to)
{
#ifndef USE_SSE
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
#else
	asm volatile (
		"cmpl	$0, %2		\n\t"	/* is "from" given? */
		"je	1f		\n\t"

		"pushl	$3f		\n\t"	/* EIP */
		"pushfl			\n\t"	/* EFLAGS */
		"pushal			\n\t"	/* all registers */
		"movl	%%esp, %0	\n\t"	/* save stack => from */

		"cmpl	$0, %5		\n\t"	/* is sse supported? */
		"je	2f		\n\t"
		"cmpl	$0, %1		\n\t"	/* is from->sse_mmx_fpu != 0 */
		"je	1f		\n\t"

		/* store SSE context */
		"sub	$512, %%esp	\n\t" /* reserve space on stack */
		"shr	$4, %%esp	\n\t" /* align it on 16 byte boundary */
		"shl	$4, %%esp	\n\t"
		"fxsave	(%%esp)		\n\t"
		"movl	%%esp, %1	\n\t"

		/* restore SSE context */
	"1:	 cmpl	$0, %4		\n\t"	/* is to->sse_mmx_fpu != 0 */
		"je	2f		\n\t"

		"movl	 %4, %%ebx	\n\t"
		"fxrstor	(%%ebx)	\n\t"

	"2:	 movl	%3, %%esp	\n\t"	/* restore stack <= to*/
		"popal			\n\t"
		"popfl			\n\t"
		"ret			\n\t"

	"3:	 nop			\n\t"	/* threads restarts here */

		: "=m" (from->context),		/* %0 */
		  "=m" (from->sse_mmx_fpu)	/* %1 */
		: "m" (from),			/* %2 */
		  "m" (to->context),		/* %3 */
		  "m" (to->sse_mmx_fpu),	/* %4 */
		  "m" (arch_sse_supported)	/* %5 */
		: "%ebx"
	);
#endif
}

/* switch stack and call thread_exit2 */
void arch_thread_exit_with_stack_switch(void *kthread, void *exit_status, int force)
{
	extern uint8 system_stack[];

	asm volatile (
	/* save parameters from stack to registers */
	"movl	%0, %%eax	\n\t"
	"movl	%1, %%ebx	\n\t"
	"movl	%2, %%ecx	\n\t"

	"movl	%3, %%esp	\n\t"	/* switch to bootup stack */

	/* push parameters from registers to stack */
	"pushl	%%ecx		\n\t"
	"pushl	%%ebx		\n\t"
	"pushl	%%eax		\n\t"

	"call	kthread_exit2	\n\t"
	/* there is no return to here */

		:: "m" (kthread), 				/* %0 */
		   "m" (exit_status),				/* %1 */
		   "m" (force),					/* %2 */
		   "g" (system_stack + KERNEL_STACK_SIZE)	/* %3 */
		: "%eax", "%ebx", "%ecx"
	);
}
