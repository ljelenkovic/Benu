/*! Segment descriptor tables: GDT, IDT, TSS */

#define _ARCH_DESCRIPTORS_C_
#include "descriptor.h"

#include <arch/interrupt.h>
#include <kernel/errno.h>

/*! memory for GDT - Global Descriptor Table */
static GDT_t gdt[] =
{
	GDT_0,
	GDT_K_CODE, GDT_K_DATA,
	GDT_T_CODE, GDT_K_DATA,
	GDT_TSS
};

/*! IDT */
static IDT_t idt[INTERRUPTS];

/*! memory for TSS */
static tss_t tss;


/*! Set up context (normal and interrupt=kernel) */
void arch_descriptors_init()
{
	GDT_init();
	IDT_init();
}

/*! Set up GDT */
static void GDT_init()
{
	GDTR_t gdtr;

	/* initial update of segment descriptors */
	arch_upd_segm_descr(SEGM_K_CODE, NULL, (size_t) 0xffffffff, PRIV_KERNEL);
	arch_upd_segm_descr(SEGM_K_DATA, NULL, (size_t) 0xffffffff, PRIV_KERNEL);
	arch_upd_segm_descr(SEGM_T_CODE, NULL, (size_t) 0xffffffff, PRIV_USER);
	arch_upd_segm_descr(SEGM_T_DATA, NULL, (size_t) 0xffffffff, PRIV_USER);
	arch_upd_segm_descr(SEGM_TSS, &tss, sizeof(tss_t) - 1, PRIV_KERNEL);

	gdtr.gdt = gdt;
	gdtr.limit = sizeof(gdt) - 1;

	/* load GDT address into register */
	asm ("lgdt	%0\n\t" :: "m" (gdtr));

	/* update segment registers */
	asm volatile (
			"ljmp	%0, $1f	\n"	/* update "cs" */
		/* $1f = '1'=local label, 'f'='forward' (first '1') after */
		"1:				\n\t"
			"mov	%1, %%eax	\n\t"
			"mov	%%ax, %%ds	\n\t"
			"mov	%%ax, %%es	\n\t"
			"mov	%%ax, %%fs	\n\t"
			"mov	%%ax, %%gs	\n\t"
			"mov	%%ax, %%ss	\n\t"

		:: "i" (GDT_DESCRIPTOR(SEGM_K_CODE, GDT, PRIV_KERNEL)),
		   "i" (GDT_DESCRIPTOR(SEGM_K_DATA, GDT, PRIV_KERNEL))
		: "memory"
	);

	tss.ss0 = GDT_DESCRIPTOR(SEGM_K_DATA, GDT, PRIV_KERNEL);
	/* load TSS descriptor into TR */
	asm ("ltrw %w0\n\t" :: "r" GDT_DESCRIPTOR(SEGM_TSS, GDT, PRIV_KERNEL));
}

/*! Set up IDT */
static void IDT_init()
{
	/*! Interrupt handlers are defined in 'interrupts.S' */
	extern uint32 arch_interrupt_handlers[INTERRUPTS];
	static IDTR_t idtr;
	int i;
	uint32 offset;

	/* fill IDT */
	for (i = 0; i < INTERRUPTS; i++)
	{
		offset = (uint32) arch_interrupt_handlers[i];

		idt[i].offset_lo = offset & 0xffff;
		idt[i].seg_sel	= GDT_DESCRIPTOR(SEGM_K_CODE, GDT, PRIV_KERNEL);
		idt[i].zero	= 0;
		idt[i].type	= 0x0e;
		idt[i].DPL	= PRIV_KERNEL;
		idt[i].present	= 1;
		idt[i].offset_hi = (offset >> 16) & 0xffff;
	}

	/* threads should be able to generate software interrupt */
	idt[SOFTWARE_INTERRUPT].DPL = PRIV_USER;

	/* load table address into register */
	idtr.base = idt;
	idtr.limit = sizeof(IDT_t) * INTERRUPTS - 1;
	asm ("lidt %0" : : "m" (idtr));
}

/*! Update segment descriptor with starting address, size and privilege level */
void arch_upd_segm_descr(int id, void *start_addr, size_t size,
				  int priv_level)
{
	uint32 addr = (uint32) start_addr;
	uint32 gsize = size;

	ASSERT(id > 0 && id <= 5);

	gdt[id].base_addr0 =  addr & 0x0000ffff;
	gdt[id].base_addr1 = (addr & 0x00ff0000) >> 16;
	gdt[id].base_addr2 = (addr & 0xff000000) >> 24;

	if (size < (1 << 20)) { /* size < 1 MB? */
		gsize = size - 1;
		gdt[id].G = 0; /* granularity set to 1 byte */
	}
	else {
		gsize = size >> 12;
		if (size & 0x0fff)
			gsize++;
		gsize--;
		gdt[id].G = 1; /* granularity set to 4 KB */
	}

	gdt[id].segm_limit0 =  gsize & 0x0000ffff;
	gdt[id].segm_limit1 = (gsize & 0x000f0000) >> 16;

	gdt[id].DPL = priv_level;
}

/*!
 * Prepare TSS for next thread
 * - update pointer for next thread context, where to save it on interrupt
 */
void arch_tss_update(void *context)
{
	tss.esp0 = (uint32) context;
}
