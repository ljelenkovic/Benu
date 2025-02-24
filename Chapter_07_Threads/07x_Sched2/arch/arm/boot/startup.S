/*! startup.S - starting point of control */

#define ASM_FILE	1

#include <arch/multiboot.h>

/* stack, startup function */
.extern	k_startup, arch_processor_init

.section .text

/* entry point */
.global arch_startup, arch_jump_to_kernel

/* 32 bit alignment is required for following constants */
.align 4

/* Multiboot header */
multiboot_header:
	/* magic */
	.long	MULTIBOOT_HEADER_MAGIC
	/* flags */
	.long	MULTIBOOT_HEADER_FLAGS
	/* checksum */
	.long	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

/* THE starting point */
arch_startup:
	/* initialize processor(operating mode, stacks, interrupt subsystem) */
	bl	arch_processor_init

arch_jump_to_kernel:
	/* call starting kernel function */
	bl	k_startup

	/* stop */
	mrs	r0, cpsr
	orr	r0, r0, #0x1c0	/* disable interrupts */
	msr	cpsr, r0
	b	.		/* infinite loop */
