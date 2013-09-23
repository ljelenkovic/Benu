/*! interrupts.S - low level (arch) interrupt handling */
#define ASM_FILE	1

#include "descriptor.h"

/* defined in arch/context.c */
.extern arch_thr_context, arch_thr_context_ss, arch_interrupt_stack

/* defined in kernel/interrupts.c */
.extern arch_interrupt_handler

/* Interrupt handlers function addresses, required for filling IDT */
.globl arch_interrupt_handlers
.globl arch_return_to_thread

#ifdef USE_SSE
.extern arch_sse_supported, arch_sse_mmx_fpu
#endif


.section .text


/* Interrupt handlers
 * - save all register, save interrupt number and jump to common stub
 * - implemented via macro (for each interrupt number we are handling)
 */
.irp int_num,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,\
	25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48
.type interrupt_\int_num, @function

interrupt_\int_num:

.if \int_num < 8 || \int_num == 9 || \int_num > 14
	pushl	$0	/* dummy error code when real is not provided */
.endif

	pushal

	movl	$\int_num, %eax
	jmp	.arch_interrupts_common_routine
.endr


/* Main 'arch' interrupt handler routine
 * - after initial context save, all interrupt handlers continue with this code
 * - perform required 'housekeeping' operations and forward processing to
 *   C code function in arch layer (interrupts.c: arch_interrupt_handler)
 */
.arch_interrupts_common_routine:

	/* save thread segment registers in thread context */
	pushw	%ds
	pushw	%es
	pushw	%fs
	pushw	%gs

	/* activate interrupt (kernel) segments and stack */
	mov	$GDT_DESCRIPTOR ( SEGM_K_DATA, GDT, PRIV_KERNEL ), %bx
	mov	%bx, %ds
	mov	%bx, %es
	mov	%bx, %fs
	mov	%bx, %gs
	mov	%bx, %ss
	movl	arch_interrupt_stack, %esp

#ifdef USE_SSE
	cmpl	$0, arch_sse_supported	/* check if SSE is supported */
	je	.noSSE1
	movl	arch_sse_mmx_fpu, %ebx
	fxsave	(%ebx)
.noSSE1:
#endif

	/* save interrupt number on stack - arg. for int. handling function */
	pushl	%eax

	/* forward further processing to 'arch' layer
	   (device driver or forward call to kernel) */
	call	arch_interrupt_handler

#ifdef USE_SSE
	cmpl	$0, arch_sse_supported	/* check if SSE is supported */
	je	.noSSE2
	movl	arch_sse_mmx_fpu, %ebx
	fxrstor	(%ebx)
.noSSE2:
#endif

arch_return_to_thread:
/* label used for switch from initial boot up thread to 'normal' threads */

	/* restore stack segment where thread context is saved */
	movw	arch_thr_context_ss, %ss
	/* restore pointer where thread context is saved */
	movl	arch_thr_context, %esp

	/* restore 'context' */
	/* restore thread segment registers from thread context */
	popw	%gs
	popw	%fs
	popw	%es
	popw	%ds

	popal

	/* remove error code (either real or dummy) from stack */
	addl	$4, %esp

	/* return from interrupt to thread (restore eip, cs, eflags) */
	iret

.section .data
.align	4

/* Interrupt handlers function addresses, required for filling IDT */
.type	arch_interrupt_handlers, @object
.size	arch_interrupt_handlers, 49*4

arch_interrupt_handlers:
.irp int_num,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,\
	25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48

	.long interrupt_\int_num
.endr
	.long 0
