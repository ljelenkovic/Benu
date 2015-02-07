/*! startup.S - starting point of control after grub (boot loader) */

/*! Multiboot constants (basic) */
#define MULTIBOOT_HEADER_MAGIC	0x1BADB002
#define MULTIBOOT_HEADER_FLAGS	0

/* stack, startup function */
.extern	clock

/* this code must be first in image for boot loader to find it easy */
.section .text

/* entry point (required for boot loader) */
.global arch_start

/* 32 bit alignment is required for following constants */
.align  4

/* Multiboot header */
multiboot_header:
	/* magic */
	.long	MULTIBOOT_HEADER_MAGIC
	/* flags */
	.long	MULTIBOOT_HEADER_FLAGS
	/* checksum */
	.long	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

/* THE starting point */
arch_start:
	/* stack pointer initialization */
	mov	$stack, %esp

	/* starting status register - EFLAGS register */
	pushl	$0
	popf

	call	clock

	/* stop: disable interrupts and suspend processor */
	cli
loop:   hlt
	jmp	loop


.section .bss
.align  4

	.lcomm _stack_, 4096
stack:
