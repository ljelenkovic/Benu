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

extern MEM_ALLOC_T *k_mpool; /* defined in *ff_simple.c, *gma.h, ... */


/*! Kernel memory layout ---------------------------------------------------- */
#include <types/basic.h>
#include <lib/list.h>
#include <api/prog_info.h>
#include <arch/memory.h>

void k_memory_init();
void k_memory_info();


/*! Available (loaded) program */
struct _kprog_t_
{
	program_t  *prog;
		    /* defined as header of program */

	mseg_t     *m;
		    /* memory segment this program occupies */
};

/*! Process ----------------------------------------------------------------- */

/*! Process */
struct _kprocess_t_
{
	mseg_t	      m;
		      /* memory segment this process occupies */

	kprog_t      *prog;
		      /* link to associated program */

	process_t    *proc;
		      /* process header - at start of process memory */

	char          name[16];	/* program name */

	void         *heap; /* physical address of heap area */
	size_t        heap_size;

	void         *stack; /* physical address of stack area */
	size_t        stack_size;
	size_t        thread_stack_size;
	uint         *smap; /* bitmap for stack allocation */
	              /* allocation unit = thread_stack */
		      /* allocation units = stack_size / thread_stack */
	uint          smap_size;

	uint          prio;	/* default priority for threads */

	int	      thread_count;

	list_t	      kobjects;
		      /* kobject_t elements */
};

/*! Object referenced in process (kernel object reference + additional info) */
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

void *kmalloc_kobject(kprocess_t *proc, size_t obj_size);
void *kfree_kobject(kprocess_t *proc, kobject_t *kobj);
int   kfree_process_kobjects(kprocess_t *proc);

void *kprocess_stack_alloc(kprocess_t *kproc);
void kprocess_stack_free(kprocess_t *kproc, void *stack);
