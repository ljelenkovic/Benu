/*! Program info */
#pragma once

#include <types/basic.h>
#include <arch/memory.h>


typedef struct _prog_info_t_
{
	/* defined in compile time */
	uint32  magic[4];
		/* magic numbers to identify start of module */

	uint32  type;
		/* MS_PROGRAM, MS_OTHER */

	/* for calculating size */
	void   *start;
	void   *end;

	char    name[16];
		/* (short) module/program name or description */

	void   *init;		/* process initialization function */
	void   *entry;		/* starting user function */
	void   *param;		/* parameter to starting function */
	void   *exit;		/* terminating function */
	uint    prio;

	/* (re)defined in run time */
	void   *heap;
	size_t  heap_size;

	void   *mpool;
}
prog_info_t;

void prog_init(void *args);
