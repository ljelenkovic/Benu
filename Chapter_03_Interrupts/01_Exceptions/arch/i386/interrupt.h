/*! Interrupt handling - 'arch' layer (only basic operations) */

#pragma once

/* Constants */
#define INT_STF			12	/* Stack Fault */
#define INT_GPF			13	/* General Protection Fault */
#define SOFT_IRQ		32
#define INTERRUPTS		33

#define INT_MEM_FAULT		INT_STF
#define INT_UNDEF_FAULT		INT_GPF

#include <arch/interrupt.h>
