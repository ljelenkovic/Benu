/*! System call (syscall) interface
 *  used by kernel to retrieve syscall parameters
 */

#pragma once

#include <arch/context.h>
#include <kernel/memory.h>

/* syscall is from threads called as: int syscall(id, arg1, arg2, ...);
 *
 * parameters are in thread descriptor (top to bottom):
 *	[return addres] [id] [arg1] [arg2] ...
 *
 * thread might be in its own address space - convert addresses if required
 */

/*! Get syscall id from thread descriptor */
static inline uint arch_syscall_get_id(context_t *cntx)
{
	return U2K_GET_INT((void *)(cntx->context.esp + 1), cntx->proc);
}

/*! Get address of first parameter to syscall (not including id) */
static inline void *arch_syscall_get_params(context_t *cntx)
{
	return U2K_GET_ADR((void *)(cntx->context.esp + 2), cntx->proc);
}

/*! Save syscall return value for thread; gcc uses eax register */
static inline void arch_syscall_set_retval(context_t *cntx, int retval)
{
	cntx->context.eax = retval;
}
