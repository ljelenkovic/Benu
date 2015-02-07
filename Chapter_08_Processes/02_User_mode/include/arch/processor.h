/*! Assembler macros for some processor control instructions */
#pragma once

#include <ARCH/processor.h>

/*! disable/enable accepting of all IO interrupts */
#define disable_interrupts()	arch_disable_interrupts()
#define enable_interrupts()	arch_enable_interrupts()

/*! halt system - stop processor or end in indefinite loop, interrupts off */
#define halt()			arch_halt()

/*! suspend processor until next interrupt */
#define suspend()		arch_suspend()
#define user_mode_suspend()	arch_user_mode_suspend()

/*! raise software interrupt */
#define raise_interrupt(p)	arch_raise_interrupt(p)

/*! memory barrier */
#define memory_barrier()	arch_memory_barrier()
