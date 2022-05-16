/*! Segment descriptor tables: GDT, IDT, TSS */

#pragma once

/*! descriptor indexes */
#define SEGM_CODE	1
#define SEGM_DATA	2

#define PRIV_KERNEL	0
#define PRIV_USER	3

#define GDT	0
#define LDT	1

#define GDT_DESCRIPTOR(ID, TABLE, PRIV_LEVEL)	\
	(((ID) << 3) |((TABLE) << 2) |(PRIV_LEVEL))

#ifndef ASM_FILE

#include <types/basic.h>

void arch_descriptors_init();

#endif

#ifdef _ARCH_DESCRIPTORS_C_

/** GDT - Global Descriptor Table **/

/*! GDT row format */
typedef struct _GDT_t_
{
	uint16	segm_limit0; /* segment limit, bits: 15:00	(00-15) */
	uint16	base_addr0;  /* starting address, bits: 15:00	(16-31) */
	uint8	base_addr1;  /* starting address, bits: 23:16	(32-38) */
	uint8	type	: 4; /* segment type			(39-42) */
	uint8	S	: 1; /* type: 0-system, 1-code or data	(43-43) */
	uint8	DPL	: 2; /* descriptor privilege level	(44-45) */
	uint8	P	: 1; /* present (in memory)		(46-46) */
	uint8	segm_limit1: 4; /*segment limit, bits: 19:16	(47-50) */
	uint8	AVL	: 1; /* "Available for use"		(51-51) */
	uint8	L	: 1; /* 64-bit code?			(52-52) */
	uint8	DB	: 1; /* 1 - 32 bit system, 0 - 16 bit	(53-53) */
	uint8	G	: 1; /* granularity 0-1B, 1-4kB 	(54-54) */
	uint8	base_addr2;  /* starting address, bits: 23:16	(55-63) */
}
__attribute__((__packed__)) GDT_t;

/*! GDT register - GDTR */
typedef struct _GDTR_t_
{
	uint16 limit;	/* GDT size */
	GDT_t *gdt;	/* GDT address (location) */
}
__attribute__((__packed__)) GDTR_t;

/*! GDT elements defaults values */

/* First element (with index 0) must be zero */
#define GDT_0	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

/* Second element (with index 1) describes code segment
 * Since no protection is used so far, we turn this off by allowing code from
 * any memory location, within available 4 GB (r-x)
 */
#define GDT_CODE 			\
{	0xffff,	/* segm_limit0	*/	\
	0,	/* base_addr0	*/	\
	0,	/* base_addr1	*/	\
	0x0a,	/* type	r-x	*/	\
	1,	/* S		*/	\
	0,	/* DPL - ring 0 */	\
	1,	/* P		*/	\
	0x0f,	/* segm_limit1	*/	\
	0,	/* AVL		*/	\
	0,	/* L		*/	\
	1,	/* DB		*/	\
	1,	/* G		*/	\
	0	/* base_addr2	*/	\
}

/* Third element (with index 2) describes data segment
 * Since no protection is used so far, we turn this off by allowing data from
 * any memory location, within available 4 GB (rw-)
 */
#define GDT_DATA 			\
{	0xffff,	/* segm_limit0	*/	\
	0,	/* base_addr0	*/	\
	0,	/* base_addr1	*/	\
	0x02,	/* type	rw-	*/	\
	1,	/* S		*/	\
	0,	/* DPL - ring 0 */	\
	1,	/* P		*/	\
	0x0f,	/* segm_limit1	*/	\
	0,	/* AVL		*/	\
	0,	/* L		*/	\
	1,	/* DB		*/	\
	1,	/* G		*/	\
	0	/* base_addr2	*/	\
}

static void arch_upd_segm_descr(int id, void *start, size_t size, int priv);

#endif /* _ARCH_DESCRIPTORS_C_ */
