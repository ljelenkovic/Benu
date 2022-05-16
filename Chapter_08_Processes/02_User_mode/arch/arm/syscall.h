/*! System call (syscall) interface
 *  used by kernel to retrieve syscall parameters
 */

#pragma once

#include <arch/context.h>
#include <kernel/memory.h>

/* syscall is from threads called as: int syscall(id, arg1, arg2, ...);
 *
 * parameters are on placed as follows:
 *  - first 4 in r0-r4
 *  - rest on stack
 * but since r0-r4 are first saved with thread context, all parameters are
 * together on stack
 */

/*! Get syscall id from thread stack */
static inline uint arch_syscall_get_id(context_t *cntx)
{
	return cntx->context->rl[0];
}

/*! Get address of first parameter to syscall (not including id) */
static inline void *arch_syscall_get_params(context_t *cntx)
{
	return (void *) &cntx->context->rl[1];
}

/*! Save syscall return value for thread; gcc uses eax register */
static inline void arch_syscall_set_retval(context_t *cntx, int retval)
{
	cntx->context->rl[0] = retval;
}
