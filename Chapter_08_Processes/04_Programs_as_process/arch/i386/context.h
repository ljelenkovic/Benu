/*! Processor/thread context */

#pragma once

#include <types/basic.h>
#include <arch/context.h>

#define	INIT_EFLAGS	0x3202 /* ring 3 ! */


/*! context manipulation - for 'kernel threads'  ---------------------------- */

typedef struct _arch_context_t_
{
	uint16   gs, fs, es, ds;
	int32    edi, esi, ebp, _esp, ebx, edx, ecx, eax;
	int32    err;
	uint32   eip;
	uint32   cs;
	uint32   eflags;
	uint32  *esp;	/* thread stack */
	uint32   ss;	/* thread stack segment descriptor */
}
__attribute__((__packed__)) arch_context_t;

struct _context_t_
{
	arch_context_t  context;

#ifdef USE_SSE
	uint32          sse_mmx_fpu;
	void           *sse_mmx_fpu_start;
#endif

	void           *proc; /* pointer to thread's process descriptor */
};

/*! context manipulation - for 'user threads' (in programs) ----------------- */

struct _ucontext_t_
{
	uint32  *esp;
};

static inline void arch_create_uthread_context(ucontext_t *context,
		void (func)(void *), void *param, void (thread_exit)(),
		void *stack, size_t stack_size)
{
	context->esp = stack + stack_size;

	*(--context->esp) = (uint32) param;
	*(--context->esp) = (uint32) thread_exit;
	*(--context->esp) = (uint32) func;

	*(--context->esp) = INIT_EFLAGS;	/* EFLAGS register */

	/* set initial values for all general purpose registers (8) on stack */
	*(--context->esp) = 0;	/* EAX */
	*(--context->esp) = 0;	/* ECX */
	*(--context->esp) = 0;	/* EDX */
	*(--context->esp) = 0;	/* EBX */
	*(--context->esp) = 0;	/* ESP */
	*(--context->esp) = 0;	/* EBP */
	*(--context->esp) = 0;	/* ESI */
	*(--context->esp) = 0;	/* EDI */

	/* stack (context->esp) is now ready for 'context_restore' function */
}

static inline void arch_switch_to_uthread(ucontext_t *from, ucontext_t *to)
{
	asm volatile (
		"	pushl $1f\n"
		"	pushfl\n"
		"	pushal\n"
		"	mov %%esp, %0\n"

		"	mov %1, %%esp\n"

		"	popal\n"
		"	popfl\n"
		"	ret\n"
		"1:	nop\n"

		: "=m" (from->esp) : "m" (to->esp)
	);
}

#ifdef _ARCH_

#ifdef USE_SSE
/* for storing extended context: FPU, MMX, SSE */
#define SSE_CNTX_SIZE	512
#define SSE_CNTX_ALIGN	16	/* context start must be aligned */
#endif

#endif /* _ARCH_ */
