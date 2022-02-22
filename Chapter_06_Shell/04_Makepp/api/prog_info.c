/*! Program info */

#include <api/prog_info.h>

#include <api/malloc.h>

extern int PROG_START_FUNC(char *args[]);

prog_info_t pi =
{
	.init = 	prog_init,
	.entry =	PROG_START_FUNC,
	.param =	NULL,

	.mpool =	NULL,
};

int _errno;	/* Error number that represent last syscall error */

int stdio_init(); /* implemented in stdio.c */

/*! Initialize user process environment */
void prog_init(void *args)
{
	/* open stdin & stdout */
	stdio_init();

	/* initialize dynamic memory */
	pi.mpool = mem_init(pi.heap, pi.heap_size);

	/* call starting function */
	((void (*)(void *)) pi.entry)(args);
}
