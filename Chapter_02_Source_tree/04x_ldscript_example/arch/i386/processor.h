/*! Assembler macros for some processor control instructions */

#pragma once

#define arch_halt()			asm volatile ("cli \n\t" "hlt \n\t")

#define arch_memory_barrier()		asm ("" : : : "memory")
