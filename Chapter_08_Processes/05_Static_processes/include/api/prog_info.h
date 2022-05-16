/*! Program info */
#pragma once

#include <types/basic.h>

void prog_init(void *args);

/*!
 * Parameters that define a program (on disk, in memory, before starting)
 * Its used as a header for program (that include instructions, constants, data)
 */
typedef struct _program_t_
{
	/* start is the same as in module_t */
	uint32  magic[4];
	uint32  type;

	/* defined in compile time */
	void   *start;
		/* memory address for which this program was prepared,
		 * starting address (physical or logical) of this program */
	void   *end; /* for calculating size */

	char    name[16];	/* program name */
	char   *help_msg;	/* basic program information */

	void   *init;		/* process initialization function */
	void   *entry;		/* starting user function */
	void   *param;		/* parameter to starting function */
	void   *exit;		/* terminating function */
	uint    prio;   	/* default priority for threads */

	/* Sizes defined in config.ini, per program */
	size_t  heap_size;
	size_t  stack_size;
	size_t  thread_stack;
}
program_t;

/*!
 * Process runtime parameters, required from within process (its threads)
 * (not kernel descriptor!)
 */
typedef struct _process_t_
{
	program_t p;

	void   *heap;
	void   *stack;
	void   *mpool;

	//void   *heap_brk;

	/*
	 * could put other process data here (accessible to program)
	 * e.g file descriptors
	 */
}
process_t;

/*
 * Memory map of program: (addresses grows downward!)
 * +--------------------------------------------------------------------------+
 * |                       program_t header + process_t exp.                  |
 * +--------------------------------------------------------------------------+
 * |                .text, .*data*, .bss, ... (compiled sections)             |
 * +--------------------------------------------------------------------------+
 *
 * Heap and stack are added when program is started and becomes process
 */
