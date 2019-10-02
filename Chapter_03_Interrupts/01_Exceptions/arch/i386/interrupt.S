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
 *
 * NOTE: kernel functions should not use SSE/MMX/FPU
 * (or should save previous state and restore it afterwards)
 */

.type interrupt_0, @function
interrupt_0:
	pushl	$0		/* dummy error code when real is not provided */
	pushal			/* save 'context' (general registers) */

	pushl	$0		/* push interrupt number on stack */
	call	arch_interrupt_handler
	addl	$4, %esp	/* remove interrupt number from stack */

	popal			/* restore 'context' */
	addl	$4, %esp     /* remove error code (real or dummy) from stack */

	iret	/* return from interrupt to thread (restore eip, cs, eflags) */

.type interrupt_1, @function
interrupt_1:
	pushl	$0
	pushal

	pushl	$1
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_2, @function
interrupt_2:
	pushl	$0
	pushal

	pushl	$2
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_3, @function
interrupt_3:
	pushl	$0
	pushal

	pushl	$3
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_4, @function
interrupt_4:
	pushl	$0
	pushal

	pushl	$4
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_5, @function
interrupt_5:
	pushl	$0
	pushal

	pushl	$5
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_6, @function
interrupt_6:
	pushl	$0
	pushal

	pushl	$6
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_7, @function
interrupt_7:
	pushl	$0
	pushal

	pushl	$7
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_8, @function
interrupt_8:
	pushl	$0
	pushal

	pushl	$8
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_9, @function
interrupt_9:
	pushl	$0
	pushal

	pushl	$9
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_10, @function
interrupt_10:
	pushl	$0
	pushal

	pushl	$10
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_11, @function
interrupt_11:
	pushl	$0
	pushal

	pushl	$11
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_12, @function
interrupt_12:
	pushl	$0
	pushal

	pushl	$12
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_13, @function
interrupt_13:
	pushl	$0
	pushal

	pushl	$13
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_14, @function
interrupt_14:
	pushl	$0
	pushal

	pushl	$14
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_15, @function
interrupt_15:
	pushl	$0
	pushal

	pushl	$15
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_16, @function
interrupt_16:
	pushl	$0
	pushal

	pushl	$16
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_17, @function
interrupt_17:
	pushl	$0
	pushal

	pushl	$17
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_18, @function
interrupt_18:
	pushl	$0
	pushal

	pushl	$18
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_19, @function
interrupt_19:
	pushl	$0
	pushal

	pushl	$19
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_20, @function
interrupt_20:
	pushl	$0
	pushal

	pushl	$20
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_21, @function
interrupt_21:
	pushl	$0
	pushal

	pushl	$21
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_22, @function
interrupt_22:
	pushl	$0
	pushal

	pushl	$22
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_23, @function
interrupt_23:
	pushl	$0
	pushal

	pushl	$23
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_24, @function
interrupt_24:
	pushl	$0
	pushal

	pushl	$24
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_25, @function
interrupt_25:
	pushl	$0
	pushal

	pushl	$25
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_26, @function
interrupt_26:
	pushl	$0
	pushal

	pushl	$26
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_27, @function
interrupt_27:
	pushl	$0
	pushal

	pushl	$27
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_28, @function
interrupt_28:
	pushl	$0
	pushal

	pushl	$28
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_29, @function
interrupt_29:
	pushl	$0
	pushal

	pushl	$29
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_30, @function
interrupt_30:
	pushl	$0
	pushal

	pushl	$30
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_31, @function
interrupt_31:
	pushl	$0
	pushal

	pushl	$31
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_32, @function
interrupt_32:
	pushl	$0
	pushal

	pushl	$32
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_33, @function
interrupt_33:
	pushl	$0
	pushal

	pushl	$33
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_34, @function
interrupt_34:
	pushl	$0
	pushal

	pushl	$34
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_35, @function
interrupt_35:
	pushl	$0
	pushal

	pushl	$35
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_36, @function
interrupt_36:
	pushl	$0
	pushal

	pushl	$36
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_37, @function
interrupt_37:
	pushl	$0
	pushal

	pushl	$37
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_38, @function
interrupt_38:
	pushl	$0
	pushal

	pushl	$38
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_39, @function
interrupt_39:
	pushl	$0
	pushal

	pushl	$39
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_40, @function
interrupt_40:
	pushl	$0
	pushal

	pushl	$40
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_41, @function
interrupt_41:
	pushl	$0
	pushal

	pushl	$41
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_42, @function
interrupt_42:
	pushl	$0
	pushal

	pushl	$42
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_43, @function
interrupt_43:
	pushl	$0
	pushal

	pushl	$43
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_44, @function
interrupt_44:
	pushl	$0
	pushal

	pushl	$44
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_45, @function
interrupt_45:
	pushl	$0
	pushal

	pushl	$45
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_46, @function
interrupt_46:
	pushl	$0
	pushal

	pushl	$46
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_47, @function
interrupt_47:
	pushl	$0
	pushal

	pushl	$47
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret

.type interrupt_48, @function
interrupt_48:
	pushl	$0
	pushal

	pushl	$48
	call	arch_interrupt_handler
	addl	$4, %esp

	popal
	addl	$4, %esp

	iret


.section .data
.align	4

/* Interrupt handlers function addresses, required for filling IDT */
.type	arch_interrupt_handlers, @object
.size	arch_interrupt_handlers, 49*4

arch_interrupt_handlers:
	.long interrupt_0
	.long interrupt_1
	.long interrupt_2
	.long interrupt_3
	.long interrupt_4
	.long interrupt_5
	.long interrupt_6
	.long interrupt_7
	.long interrupt_8
	.long interrupt_9
	.long interrupt_10
	.long interrupt_11
	.long interrupt_12
	.long interrupt_13
	.long interrupt_14
	.long interrupt_15
	.long interrupt_16
	.long interrupt_17
	.long interrupt_18
	.long interrupt_19
	.long interrupt_20
	.long interrupt_21
	.long interrupt_22
	.long interrupt_23
	.long interrupt_24
	.long interrupt_25
	.long interrupt_26
	.long interrupt_27
	.long interrupt_28
	.long interrupt_29
	.long interrupt_30
	.long interrupt_31
	.long interrupt_32
	.long interrupt_33
	.long interrupt_34
	.long interrupt_35
	.long interrupt_36
	.long interrupt_37
	.long interrupt_38
	.long interrupt_39
	.long interrupt_40
	.long interrupt_41
	.long interrupt_42
	.long interrupt_43
	.long interrupt_44
	.long interrupt_45
	.long interrupt_46
	.long interrupt_47
	.long interrupt_48
	.long 0
