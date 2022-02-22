/*! Program info */

#include <api/prog_info.h>
#include <arch/memory.h>

#include <api/pthread.h>
#include <api/malloc.h>

/* symbols from user.ld */
extern char user_code, user_end;

extern int PROG_START_FUNC(char *args[]);
extern char PROG_HELP[];

module_program_t _program_module_header_
__attribute__((section(".program_header"))) =
{
	.prog = {
		.magic =	{ PMAGIC1, ~PMAGIC1, PMAGIC2, ~PMAGIC2 },
		.type = 	MS_PROGRAM,

		.start =	&user_code,
		.end =		&user_end,

		.name =		PROG_NAME,
		.help_msg =	PROG_HELP,

		.init =		prog_init,
		.entry =	PROG_START_FUNC,
		.param =	NULL,
		.exit =		pthread_exit,
		.prio =		THR_DEFAULT_PRIO,

		.heap_size =	HEAP_SIZE,
		.stack_size =	STACK_SIZE,
		.thread_stack =	THREAD_STACK_SIZE,
	}
};

/* used in runtime */
process_t *_uproc_;

int stdio_init(); /* implemented in stdio.c */

/*! Initialize process environment */
void prog_init(void *args)
{
	_uproc_ = &_program_module_header_.proc;

	/* open stdin & stdout */
	stdio_init();

	/* initialize dynamic memory */
	_uproc_->mpool = mem_init(_uproc_->heap, _uproc_->p.heap_size);

	/* call starting function */
	((void (*)(void *)) _uproc_->p.entry)(args);

	pthread_exit(NULL);
}
