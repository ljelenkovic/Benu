/*! Assembler macros for some processor control instructions */

#pragma once

#define arch_disable_interrupts()	\
asm (	"push {r0}\n\t"			\
	"mrs r0, cpsr\n\t"		\
	"orr r0, r0, #0xc0\n\t"		\
	"msr cpsr, r0\n\t"		\
	"pop {r0}\n\t")

#define arch_enable_interrupts()	\
asm (	"push {r0}\n\t"			\
	"mrs r0, cpsr\n\t"		\
	"bic r0, r0, #0xc0\n\t"		\
	"msr cpsr, r0\n\t"		\
	"pop {r0}\n\t")

#define arch_halt()			\
do {					\
	arch_disable_interrupts();	\
	asm volatile ("b .\n\t");	\
} while (0)
