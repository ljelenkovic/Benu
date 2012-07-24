/*! Memory management */
#define _K_MEMORY_C_

#include "memory.h"

#include "kprint.h"
#include <kernel/errno.h>
#include <arch/processor.h>
#include <arch/memory.h>

/*! Memory segments */
static mseg_t *mseg = NULL;

/*! Initial memory layout created in arch layer */
void k_memory_init ()
{
	mseg = arch_memory_init ();
}

/*! print memory layout */
void k_memory_info ()
{
	int i;

	kprintf ( "Memory segments\n"
		 "===============\n"
		 "Type\tsize\tstart addres\tstring\n"
	);

	for ( i = 0; mseg[i].type != MS_END && i < 20; i++ )
	{
		kprintf ( "%d\t%x\t%x\t%s\n", mseg[i].type, mseg[i].size,
					      mseg[i].start, mseg[i].name );
	}
}

/*! Handle memory fault interrupt */
void k_memory_fault ()
{
	LOG ( ERROR, "General Protection Fault!!!");
	halt ();
}
