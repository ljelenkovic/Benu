/*! Processor context, segment descriptor tables: GDT, IDT */

#define _ARCH_
#include <arch/memory.h>
#include "descriptor.h"
#include <arch/multiboot.h>
#include <arch/processor.h>
#include <kernel/errno.h>

/*! kernel (interrupt) stack */
uint8 system_stack [ STACK_SIZE ];

/*! Multiboot magic number and address where multiboot structure is saved */
unsigned long arch_mb_magic, arch_mb_info;

#define ALIGN_TO	4096 /* align segments to it */

static mseg_t *arch_memseg_from_script ();

/*!
 * Set up context (normal and interrupt=kernel)
 *
 * Create memory map:
 * - find place for heap
 */
mseg_t *arch_memory_init ()
{
	extern char kernel_code_addr, kernel_end_addr;
	multiboot_info_t *mbi;
	multiboot_memory_map_t *mmap, *iter;
	mseg_t *mseg;
	uint32 mmap_end, mmap_size;
	uint32 heap_id, heap_start, heap_end;
	int mmap_cnt, mseg_cnt, i;

	/* Is system booted by a Multiboot-compliant boot loader? */
	if ( arch_mb_magic != MULTIBOOT_BOOTLOADER_MAGIC )
	{
		/* no Multiboot information! get them "hard way" :) */
		return arch_memseg_from_script ();
	}
	else {
		mbi = (void *) arch_mb_info; /* from multiboot info structure */

		mmap = NULL;

		/* if memory map is available use it */
		if ( mbi->flags & MULTIBOOT_INFO_MEM_MAP )
		{
			/* find largest available segment */
			iter = (void *) mbi->mmap_addr;
			mmap_end = (uint32)(mbi->mmap_addr + mbi->mmap_length);
			mmap_cnt = 0;
			mmap_size = 0;
			heap_id = 0;
			while ( iter && (uint32) iter < mmap_end )
			{
				mmap_cnt++;

				if ( iter->type == MULTIBOOT_MEMORY_AVAILABLE
				     && mmap_size < (uint32)
				     ( iter->len & 0x0ffffffff ) )
				{
					heap_id = mmap_cnt;
					mmap = iter;
					mmap_size = (uint32)
						( iter->len & 0x0ffffffff );
				}

				iter = (void *) iter + iter->size
					+ sizeof (iter->size);
			}
		}

		if ( !mmap )
			return arch_memseg_from_script ();

		heap_start = (uint32) ( mmap->addr & 0x0ffffffff );
		heap_end = heap_start + (uint32) ( mmap->len & 0x0ffffffff );

		/* if this segment contains/intersect with kernel code/data,
		 * reduce it */
		if ( heap_start >= (uint32) &kernel_code_addr &&
		     heap_start < (uint32) &kernel_end_addr )
			heap_start = (uint32) &kernel_end_addr;

		if ( heap_end >= (uint32) &kernel_code_addr &&
		     heap_end < (uint32) &kernel_end_addr )
			heap_end = (uint32) &kernel_code_addr;

		if ( heap_start >= heap_end )
		{
			LOG ( ERROR, "No space for heap!\n" );
			halt ();
		}

		/*
		 * TODO: check if some other data is saved in segment
		 * [heap_start, heap_end]
		 */

		/* "map" "mmap" segmets to "mseg" */
		mseg_cnt = mmap_cnt + 1;
		if ( heap_start != (uint32) ( mmap->addr & 0x0ffffffff ) )
			mseg_cnt++;

		mseg = (void *) heap_start;
		heap_start += mseg_cnt * sizeof (mseg_t);

		iter = (void *) mbi->mmap_addr;
		mmap_end = (uint32) (mbi->mmap_addr + mbi->mmap_length);
		for ( i = 0; i < mmap_cnt; i++ )
		{
			mseg[i].type = iter->type;
			mseg[i].name = "mmap segment";
			mseg[i].start = (void *) (uint32)
					( iter->addr & 0x0ffffffff );
			mseg[i].size = (uint32) ( iter->len & 0x0ffffffff );

			iter = (void *) iter + iter->size + sizeof (iter->size);
		}

		i = mmap_cnt;
		if ( mseg_cnt == mmap_cnt + 1 )
		{
			mseg[heap_id].type = MS_KHEAP;
			mseg[heap_id].name = "heap";
		}
		else {
			mseg[i].type = MS_KHEAP;
			mseg[i].name = "heap";
			mseg[i].start = (void *) heap_start;
			mseg[i].size = heap_end - heap_start;
			i++;
		}

		mseg[i].type = MS_END;

		return mseg;
	}
}

/*! Create memory map from linker script and available memory (QEMU_MEM) */
static mseg_t *arch_memseg_from_script ()
{
	mseg_t *mseg;
	extern char kernel_code_addr, kernel_end_addr;

	mseg = (void *) &kernel_end_addr;

	/* kernel segment - from kernel linker script */
	mseg[0].type = MS_KERNEL;
	mseg[0].name = "kernel";
	mseg[0].start = &kernel_code_addr;
	mseg[0].size = (uint)&kernel_end_addr - (uint)&kernel_code_addr
			+ 3 * sizeof (mseg_t);

	/* kernel heap */
	mseg[1].type = MS_KHEAP;
	mseg[1].name = "heap";
	mseg[1].start = &kernel_end_addr + 3 * sizeof (mseg_t);
	mseg[1].size = ( QEMU_MEM << 20 ) - (uint) mseg[1].start;

	mseg[2].type = MS_END;

	return mseg;
}
