/*! syscall - must be in "user space" (if different from kernel)
 *
 * "Bare function", without usual "frame": int syscall(id, arg1, arg2, ...)
 */

#define ASM_FILE	1

#include "interrupt.h"

.globl syscall

/*.section .user_code */

syscall:
	svc	#SOFT_IRQ
	mov	pc, lr
