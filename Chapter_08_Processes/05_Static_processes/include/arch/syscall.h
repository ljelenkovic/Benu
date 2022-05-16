/*! System call (syscall) interface
 *  used by kernel to retrieve syscall parameters
 */
#pragma once

#include <arch/context.h>

/* syscall is from threads called as: int syscall(id, arg1, arg2, ...);
 *
 * parameters are on thread stack (top to bottom):
 *	[return address] [id] [arg1] [arg2] ...
 *
 * thread might be in its own address space - convert addresses if required
 */

/*! Get syscall id from thread stack */
static inline uint arch_syscall_get_id(context_t *cntx);

/*! Get address of first parameter to syscall (not including id) */
static inline void *arch_syscall_get_params(context_t *cntx);

/*! Save syscall return value for thread; gcc uses eax register */
static inline void arch_syscall_set_retval(context_t *cntx, int retval);

#include <ARCH/syscall.h>
