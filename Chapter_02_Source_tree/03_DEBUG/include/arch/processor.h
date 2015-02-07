/*! Assembler macros for some processor control instructions */
#pragma once

#include <ARCH/processor.h>

/*! halt system - stop processor or end in indefinite loop, interrupts off */
#define halt()			arch_halt()
