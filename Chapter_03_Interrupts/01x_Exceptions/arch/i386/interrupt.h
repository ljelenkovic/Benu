/*! Interrupt handling - 'arch' layer (only basic operations) */

#pragma once

#include "types.h"

/* Constants */
#define INT_STF			12	/* Stack Fault */
#define INT_GPF			13	/* General Protection Fault */
#define SOFT_IRQ		32
#define INTERRUPTS		33

#define INT_MEM_FAULT		INT_STF
#define INT_UNDEF_FAULT		INT_GPF

struct interrupt_frame
{
	arch_word_t ip;
	arch_word_t cs;
	arch_word_t flags;
};

#include <arch/interrupt.h>
