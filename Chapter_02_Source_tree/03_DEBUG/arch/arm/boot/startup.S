/*! startup.S - starting point of control */

#define ASM_FILE	1

#include <arch/multiboot.h>

/* stack, startup function */
.extern	system_stack, k_startup

.section .text

/* entry point */
.global arch_startup

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
	/* stack pointer initialization */
	ldr	sp, =system_stack
	add	sp, sp, #STACK_SIZE

	/* call starting kernel function */
	bl	k_startup

	/* stop */
	mrs	r0, cpsr
	orr	r0, r0, #0x1c0	/* disable interrupts */
	msr	cpsr, r0
	b	.		/* infinite loop */
