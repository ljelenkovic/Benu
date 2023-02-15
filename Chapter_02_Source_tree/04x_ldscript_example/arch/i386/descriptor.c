/*! Segment descriptor tables: GDT, IDT, TSS */

#define _ARCH_DESCRIPTORS_C_
#include "descriptor.h"

#include <kernel/errno.h>

/*! memory for GDT - Global Descriptor Table */
static GDT_t gdt[] =
{
	GDT_0,
	GDT_CODE, GDT_DATA
};

/*! Set up context */
void arch_descriptors_init()
{
	GDTR_t gdtr;

	/* initial update of segment descriptors */
	arch_upd_segm_descr(SEGM_CODE, NULL, (size_t) 0xffffffff, PRIV_KERNEL);
	arch_upd_segm_descr(SEGM_DATA, NULL, (size_t) 0xffffffff, PRIV_KERNEL);

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

		:: "i" (GDT_DESCRIPTOR(SEGM_CODE, GDT, PRIV_KERNEL)),
		   "i" (GDT_DESCRIPTOR(SEGM_DATA, GDT, PRIV_KERNEL))
		: "memory"
	);
}

/*! Update segment descriptor with starting address, size and privilege level */
static void arch_upd_segm_descr(int id, void *start_addr, size_t size,
				  int priv_level)
{
	uint32 addr = (uint32) start_addr;
	uint32 gsize = size;

	ASSERT(id > 0 && id <= 3);

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
