/*! context switching */

.extern system_stack
.global arch_switch_to_thread
.global arch_thread_exit_with_stack_switch

.section .text
.align	4

arch_switch_to_thread:
	/* r0 = from, r1 = to */
	cmp	r0, #0		/* is "from" given? */
	beq	restore

	/* save context of current thread */
	sub	sp, sp, #4	/* pc will be saved here */

	push	{r0-r12,lr}	/* registers */

	ldr	r5, =return	/* return there */
	str	r5, [sp, #56]	/* store it below 14 regs */

	mrs	r5, cpsr
	push	{r5}		/* cpsr */

	str	sp, [r0]	/* save stack address to from->context */

restore:
	ldr	r1, [r1]	/* load stack address from to->context */
	mov	sp, r1		/* restore stack <= to*/

	pop	{r5}
	msr	cpsr, r5	/* restore cpsr */

	pop	{r0-r12,lr}	/* registers */

	pop	{pc}		/* 1st time=thread func,
				   other times: addr. of next instr.(2) */

return:
	mov	pc, lr		/* return from function */


/*
 * Switch from one thread to another with cleanup of first
 * 1st param {r0} from - context of thread to delete
 * 2nd param {r1} to - context of thread to switch to
 */

_system_stack_:		.word	system_stack + KERNEL_STACK_SIZE

arch_thread_exit_with_stack_switch:
	/* r0 = kthread, r1 = exit_status, r2 = force */
	/* temporary switch to bootup stack(thread's own will be destroyed) */
	ldr	sp, _system_stack_

	/* call kthread_exit2 function */
	b	kthread_exit2 /* there is no return to here */
