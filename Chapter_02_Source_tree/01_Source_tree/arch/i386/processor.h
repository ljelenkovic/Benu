/*! Assembler macros for some processor control instructions */

#pragma once

#define arch_halt()			asm volatile ( "cli \n\t" "hlt \n\t" )

#define arch_memory_barrier()		asm ("" : : : "memory")

#include <ARCH/drivers/acpi_power_off.h>
#define arch_power_off()			\
do {						\
	acpiPowerOff ();			\
	arch_halt (); /* if acpi fails */	\
} while (0)
