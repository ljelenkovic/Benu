/*! Memory management */
#pragma once

#include <kernel/memory.h>

/*! Kernel dynamic memory --------------------------------------------------- */
#include <lib/ff_simple.h>
#include <lib/gma.h>

#if MEM_ALLOCATOR == FIRST_FIT

#define MEM_ALLOC_T ffs_mpool_t
#define	K_MEM_INIT(segment, size)	ffs_init(segment, size)
#define	KMALLOC(size)			ffs_alloc(k_mpool, size)
#define	KFREE(addr)			ffs_free(k_mpool, addr)

#elif MEM_ALLOCATOR == GMA

#define MEM_ALLOC_T gma_t
#define	K_MEM_INIT(segment, size)	gma_init(segment, size, 32, 0)
#define	KMALLOC(size)			gma_alloc(k_mpool, size)
#define	KFREE(addr)			gma_free(k_mpool, addr)

#else /* memory allocator not selected! */

#error	Dynamic memory manager not defined!

#endif

/*! Kernel memory layout ---------------------------------------------------- */
#include <types/basic.h>
#include <lib/list.h>
#include <api/prog_info.h>
#include <arch/memory.h>


extern MEM_ALLOC_T *k_mpool; /* defined in memory.c */

void k_memory_init();
void k_memory_info();

void k_memory_fault(); /* memory fault handler */
