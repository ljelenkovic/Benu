/*! context switching */

.global arch_switch_to_uthread2

.section .text
.align	4

arch_switch_to_uthread2:
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
