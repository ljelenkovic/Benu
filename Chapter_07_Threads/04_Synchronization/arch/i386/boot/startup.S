/*! startup.S - starting point of control */

#define ASM_FILE	1

#include <arch/multiboot.h>

/* stack, startup function */
.extern system_stack, k_startup, arch_descriptors_init

#ifdef USE_SSE
.extern arch_sse_supported
#endif

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
	mov	$(system_stack + KERNEL_STACK_SIZE), %esp

	/* starting status register - EFLAGS register */
	pushl	$0
	popf

#ifdef USE_SSE /* Use SSE? Enable its use */
	/* code copied from: http://wiki.osdev.org/SSE */

	/* Checking for SSE */
	movl	$0x1, %eax
	cpuid
	testl	$0x02000000, %edx
	jz	.noSSE
	/* SSE is available */
	movl	$1, arch_sse_supported

	/* enable SSE */
	movl	%cr0,	%eax
	and	$0xFFFB,%ax	/* clear co-processor emulation CR0.EM */
	or	$0x2,	%ax	/* set co-processor monitoring  CR0.MP */
	movl	%eax,	%cr0
	movl	%cr4,	%eax
	or	$0x600,	%ax	/* set CR4.OSFXSR and CR4.OSXMMEXCPT */
	movl	%eax,	%cr4	/* at the same time */
.noSSE:
#endif

	/* set up GDT, IDT */
	call	arch_descriptors_init

	/* call starting kernel function */
	call	k_startup

	/* stop */
	cli
.loop:	hlt
	jmp	.loop
