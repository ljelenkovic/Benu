/*! Program info */
#pragma once

#include <types/basic.h>

typedef struct _prog_info_t_
{
	/* defined in compile time */
	uint    zero[4];	/* should not start from very beginning */
	void   *init;		/* process initialization function */
	void   *entry;		/* starting user function */
	void   *param;		/* parameter to starting function */
	void   *exit;		/* terminating function */
	uint    prio;

	void   *start_adr;
	void   *end_adr;

	void   *heap;
	size_t  heap_size;

	/* (re)defined in run time */
	void   *mpool;
}
prog_info_t;

void prog_init ( void *args );
