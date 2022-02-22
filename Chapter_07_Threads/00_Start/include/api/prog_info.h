/*! Program info */
#pragma once

#include <types/basic.h>

typedef struct _prog_info_t_
{
	/* defined in compile time */
	void   *init;		/* process initialization function */
	void   *entry;		/* starting user function */
	void   *param;		/* parameter to starting function */

	/* (re)defined in run time */
	void   *heap;
	size_t  heap_size;

	void   *mpool;
}
prog_info_t;

void prog_init(void *args);
