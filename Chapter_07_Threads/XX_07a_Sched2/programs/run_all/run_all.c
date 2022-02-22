/*! run selected programs and poweroff */

#include <stdio.h>
#include <lib/string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <arch/processor.h>

char PROG_HELP[] = "Run all compiled programs";

#include PROGRAMS

typedef struct _cmd_t_
{
	int (*func)(char *argv[]);
	char *name;
	char *descr;
}
cmd_t;

static cmd_t prog[] = PROGRAMS_FOR_SHELL;

static int start_program(int i);

int run_all(char *args[])
{
	int i, j, error = 0;
	char *progs_to_start[] = {
		"hello", "timer", "args", "uthreads", "threads", "semaphores",
		"monitors", "messages", "signals", "rr", "edf", NULL};

	for (j = 0; !error && progs_to_start[j]; j++)
		for (i = 0; !error && prog[i].func; i++)
			if (!strcmp(progs_to_start[j], prog[i].name))
				if (start_program(i))
					error = 1;

	return error;
}

static int start_program(int i)
{
	int *status, rv;
	pthread_t thr;

	printf("Starting program: %s\n"
		 "---------------------------------------\n", prog[i].name);

	rv = pthread_create(&thr, NULL, (void *) prog[i].func, NULL);
	if (!rv)
	{
		rv = pthread_join(thr, (void **) &status);
		if (rv && get_errno() != ESRCH)
		{
			printf("\npthread_join error!\n\n");
			return -1;
		}
#if 0
		/* most functions do not call pthread_exit directly so
		 * the following test will be "false negative" */
		else if (status && *status)
		{
			printf("\nProgram %s exited with error %d!\n\n",
					prog[i].name, *status);
			return -1;
		}
#endif
		else {
			printf("\nProgram %s exited successfully!\n\n",
					prog[i].name);
			return 0;
		}
	}
	else {
		printf("\nProgram: %s not started!\n", prog[i].name);
			return -1;
	}
}
