/*! startup.S - starting point of control */

#define ASM_FILE	1

#include <arch/multiboot.h>

/* stack, startup function */
.extern system_stack, k_startup, arch_descriptors_init, arch_mb_magic, arch_mb_info

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
	mov	$(system_stack + STACK_SIZE), %esp

	/* starting status register - EFLAGS register */
	pushl	$0
	popf

	/* copy multiboot parameters in arch_mb_magic and arch_mb_info */
	movl	%eax, arch_mb_magic
	movl	%ebx, arch_mb_info

	/* set up GDT, IDT */
	call	arch_descriptors_init

	/* call starting kernel function */
	call	k_startup

	/* stop */
	cli
.loop:	hlt
	jmp	.loop
