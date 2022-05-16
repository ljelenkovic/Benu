/*! Memory management */
#pragma once

#include <kernel/memory.h>

/*! Kernel dynamic memory --------------------------------------------------- */
#include <lib/ff_simple.h>
#include <lib/gma.h>

#if MEM_ALLOCATOR_FOR_KERNEL == FIRST_FIT

#define MEM_ALLOC_T ffs_mpool_t
#define	K_MEM_INIT(segment, size)	ffs_init(segment, size)
#define	KMALLOC(size)			ffs_alloc(k_mpool, size)
#define	KFREE(addr)			ffs_free(k_mpool, addr)

#elif MEM_ALLOCATOR_FOR_KERNEL == GMA

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
extern kprog_t prog;
extern list_t kobjects;

void k_memory_init();
void k_memory_info();

/*! Program, loaded as module */
struct _kprog_t_
{
	prog_info_t  *pi;
	              /* defined as header of program */

	mseg_t	     *m;
		      /* memory segment this program occupies */

	list_t	      kobjects;
		      /* kobject_t elements */
};

/*! Object referenced in programs (kernel object reference + additional info) */
struct _kobject_t_
{
	void	*kobject;
		 /* pointer to kernel object, e.g. device */
	uint	 flags;
		 /* various flags */
	void	*ptr;
		 /* pointer for extra per process info */

	list_h	 spec;
		 /* list for object purposes */

	list_h	 list;
};

/* -------------------------------------------------------------------------- */
/*! kernel ids, objects */

id_t k_new_id();
void k_free_id(id_t id);
int k_check_id(id_t id);

void k_memory_fault(); /* memory fault handler */

void *kmalloc_kobject(size_t obj_size);
void *kfree_kobject(kobject_t *kobj);
