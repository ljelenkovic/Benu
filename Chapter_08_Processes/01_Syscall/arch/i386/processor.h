/*! Assembler macros for some processor control instructions */

#pragma once

#define arch_disable_interrupts()	asm volatile ("cli\n\t")
#define arch_enable_interrupts()	asm volatile ("sti\n\t")

#define arch_halt()			asm volatile ("cli \n\t" "hlt \n\t")

#define arch_suspend()			asm volatile ("hlt \n\t")

#define arch_raise_interrupt(p)		asm volatile (	"int %0\n\t" ::	\
							"i" (p):"memory")

#define arch_memory_barrier()		asm ("" : : : "memory")

#include <arch/processor.h>
