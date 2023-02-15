/*! Interrupt handling - 'arch' layer (only basic operations) */

#pragma once

/* Constants */

/*! Interrupt sources */
#define INT_SRC_UNUSED		0
#define INT_SRC_UNDEF		1
#define INT_SRC_SWI		2
#define INT_SRC_PRE_ABORT	3
#define INT_SRC_ABORT		4
#define INT_SRC_IRQ		5
#define INT_SRC_FIQ		6
#define INT_SRC_NUM		7

#define SOFT_IRQ		INT_SRC_SWI
#define INT_MEM_FAULT		INT_SRC_ABORT
#define INT_UNDEF_FAULT		INT_SRC_UNDEF

#define INTERRUPTS	INT_SRC_NUM

#ifndef ASM_FILE
#include <arch/interrupt.h>
#endif
