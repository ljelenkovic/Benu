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

	size_t  heap_size;
	size_t  stack_size;
	size_t  thread_stack;

	char   *help_msg;	/* Basic information on program */

	void   *start_adr;
	void   *heap;
	void   *stack;
	void   *end_adr;

	/* (re)defined in run time */
	void   *mpool;
}
prog_info_t;

void prog_init ( void *args );
