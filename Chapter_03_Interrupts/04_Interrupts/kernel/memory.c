/*! Memory management */
#define _K_MEMORY_C_

#include "memory.h"

#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <arch/processor.h>
#include <lib/string.h>
#include <lib/list.h>
#include <types/bits.h>

/*! Dynamic memory allocator for kernel */
MEM_ALLOC_T *k_mpool = NULL;

/*! Memory segments */
static mseg_t *mseg = NULL;

/*! Initial memory layout created in arch layer */
void k_memory_init()
{
	int i;

	k_mpool = NULL;
	mseg = arch_memory_init();

	/* find kernel heap */
	for (i = 0; mseg[i].type != MS_END && !k_mpool; i++)
	{
		if (mseg[i].type == MS_KHEAP)
		{
			k_mpool = k_mem_init(mseg[i].start, mseg[i].size);
			break;
		}
	}

	ASSERT(k_mpool);
}

void *k_mem_init(void *segment, size_t size)
{
	return K_MEM_INIT(segment, size);
}
void *kmalloc(size_t size)
{
	return KMALLOC(size);
}
int kfree(void *chunk)
{
	return KFREE(chunk);
}

/*! print memory layout */
void k_memory_info()
{
	int i;

	kprintf("Memory segments\n"
		 "===============\n"
		 "Type\tsize\t\tstart address\n"
	);

	for (i = 0; mseg[i].type != MS_END && i < 20; i++)
	{
		kprintf("%d\t%x\t%x\n", mseg[i].type, mseg[i].size,
					  mseg[i].start);
	}
}

/*! Handle memory fault interrupt(and others undefined) */
void k_memory_fault()
{
	LOG(ERROR, "Undefined fault(exception)!!!");
	halt();
}
