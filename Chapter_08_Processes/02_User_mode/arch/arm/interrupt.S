/*! interrupts.S - low level(arch) interrupt handling */

/*
 * Used ideas from:
 * http://balau82.wordpress.com/2012/04/15/arm926-interrupts-in-qemu/
 */

#define ASM_FILE	1

#include "interrupt.h"
#include "processor.h"

.extern arch_interrupt_handler, arch_jump_to_kernel, system_stack

.global arch_processor_init, arch_return_to_thread

.section .text
.align	4

/* interrupt table copied to address 0 */
processor_interrupt_table_start:
	ldr	pc, reset_handler_addr
	ldr	pc, undef_handler_addr
	ldr	pc, swi_handler_addr
	ldr	pc, prefetch_abort_handler_addr
	ldr	pc, data_abort_handler_addr
	b	.
	ldr	pc, irq_handler_addr
	ldr	pc, fiq_handler_addr

reset_handler_addr:		.word arch_processor_init
undef_handler_addr:		.word arch_int_hndl
swi_handler_addr:		.word arch_int_hndl
prefetch_abort_handler_addr:	.word arch_int_hndl
data_abort_handler_addr:	.word arch_int_hndl
irq_handler_addr:		.word arch_int_hndl
fiq_handler_addr:		.word arch_int_hndl
processor_interrupt_table_end:


arch_processor_init:
	/* set system mode, set stack */
	msr	cpsr, #(CPSR_MODE_SYS|CPSR_IRQ)
	ldr	sp, _system_stack_

	/* stay in system mode until interrupt */

	/* copy vector table to address 0 */
	mov	r0, #0
	ldr	r1, =processor_interrupt_table_start
	ldr	r2, =processor_interrupt_table_end
1:	ldr	r3,[r1], #4
	str	r3,[r0], #4
	cmp	r1,r2
	blt	1b

	/* jump to main */
	b	arch_jump_to_kernel


_system_stack_:		.word	system_stack + KERNEL_STACK_SIZE

_arch_thr_context_:	.word	arch_thr_context


arch_int_hndl:
	/* switch back to interrupted thread stack */
	ldr	sp, _system_stack_	/* set interrupt handler stack */
	stmfd	sp, {sp}^		/* save thread stack pointer */
	ldr	sp, [sp, #-4]		/* activate thread stack */

	/* save thread context on its stack */
	stmfd	sp!, {r0-r3}
	stmfd	sp, {r4-r14}^
	sub	sp, sp, #(11*4)
	push	{lr}			/* interrupt return address */

	mrs	r0, cpsr
	mrs	r1, spsr

	push	{r0-r1}		/* cpsr, spsr */

	ldr	r2, _arch_thr_context_
	str	sp, [r2]	/* save thread context address(stack addr.) */

	/* change to system mode and handle interrupt/system call */
	msr	cpsr, #(CPSR_MODE_SYS|CPSR_IRQ)
	ldr	sp, _system_stack_	/* set kernel stack */

	/* jump to interrupt handler(r0=cpsr) */
	bl	arch_interrupt_handler

arch_return_to_thread:
	ldr	r2, _arch_thr_context_
	ldr	sp, [r2]	/* load thread context address */

	/* change to interrupt handler mode */
	pop	{r0-r1}
	mov	r2, sp
	msr	cpsr, r0
	msr	spsr, r1
	mov	sp, r2

	pop	{lr}	/* return address / start address for new thread */

	and	r0, #0x1f		/* leave only mode bits */
	cmp	r0, #CPSR_MODE_SVC	/* is it SVC interrupt? */

	ldmfd	sp, {r4-r14}^	/* restore thread context(incl. stack p.) */
	add	sp, sp, #(11*4)
	ldmfd	sp, {r0-r3}

	beq	ret_from_svc

ret_from_irq:
	subs	pc, lr, #4		/* return from: irq, fiq...*/

ret_from_svc:
	subs	pc, lr, #0		/* return from: svc */
