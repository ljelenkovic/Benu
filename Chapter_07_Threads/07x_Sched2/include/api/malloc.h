/*! Dynamic memory allocator */
#pragma once

#include <lib/ff_simple.h>
#include <lib/gma.h>
#include <api/prog_info.h>

extern prog_info_t pi;

#if MEM_ALLOCATOR_FOR_USER == FIRST_FIT

#define MEM_ALLOC_T ffs_mpool_t

#define	mem_init(segment, size)		ffs_init(segment, size)
#define	malloc(size)			ffs_alloc(pi.mpool, size)
#define	free(addr)			ffs_free(pi.mpool, addr)

#elif MEM_ALLOCATOR_FOR_USER == GMA

#define MEM_ALLOC_T gma_t

#define	mem_init(segment, size)		gma_init(segment, size, 32, 0)
#define	malloc(size)			gma_alloc(pi.mpool, size)
#define	free(addr)			gma_free(pi.mpool, addr)

#else /* memory allocator not selected! */

#define	mem_init			k_mem_init_Not_Implemented
#define	malloc				k_mem_alloc_Not_Implemented
#define	free				k_mem_free_Not_Implemented

#endif
