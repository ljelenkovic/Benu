/*! Program info */

#include <api/prog_info.h>

#include <api/pthread.h>
#include <api/malloc.h>

/* symbols from user.ld */
extern char user_code, user_end, user_heap, user_stack;

extern int PROG_START_FUNC(char *args[]);

prog_info_t pi __attribute__((section(".program_header"))) =
{
	.magic =	{ PMAGIC1, ~PMAGIC1, PMAGIC2, ~PMAGIC2 },
	.type = 	MS_PROGRAM,
	.start =	&user_code, /* from user.ld */
	.end =		&user_end,  /* from user.ld */
	.name =		PROG_START_FUNC_NAME,
	.init =		prog_init,
	.entry =	PROG_START_FUNC,
	.param =	NULL,
	.exit =		pthread_exit,
	.prio =		THR_DEFAULT_PRIO,
};

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

	pthread_exit(NULL);
}
