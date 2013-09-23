/*! interrupts.S - low level (arch) interrupt handling */
#define ASM_FILE	1

/* defined in kernel/interrupts.c */
.extern arch_interrupt_handler

/* Interrupt handlers function addresses, required for filling IDT */
.globl arch_interrupt_handlers


.section .text

/* Interrupt handlers
 * - save all register
 * - save interrupt number
 * - save thread state (processor's registers)
 * - call common interrupt handler (C code function in arch/interrupts.c)
 * - implemented via macro (for each interrupt number we are handling)
 *
 * NOTE: kernel functions should not use SSE/MMX/FPU
 * (or should save previous state and restore it afterwards)
 */

.irp int_num,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,\
	25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48
.type interrupt_\int_num, @function

interrupt_\int_num:

.if \int_num < 8 || \int_num == 9 || \int_num > 14
	pushl	$0		/* dummy error code when real is not provided */
.endif

	pushal			/* save 'context' (general registers) */

	pushl	$\int_num	/* push interrupt number on stack */

	call	arch_interrupt_handler

	addl	$4, %esp	/* remove interrupt number from stack */

	popal			/* restore 'context' */


	addl	$4, %esp     /* remove error code (real or dummy) from stack */

	iret	/* return from interrupt to thread (restore eip, cs, eflags) */
.endr

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
