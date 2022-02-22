/*! run selected programs and poweroff */

#include <stdio.h>
#include <api/prog_info.h>
#include <lib/string.h>
#include <time.h>

typedef struct _cmd_t_
{
	int (*func)(char *argv[]);
	char *name;
	char *descr;
}
cmd_t;

static cmd_t prog[] = PROGRAMS_FOR_SHELL;

int run_all(char *args[])
{
	int i, j;
	char *progs_to_start[] = {
		"hello", "timer", "args", NULL};

	for (j = 0; progs_to_start[j]; j++)
	for (i = 0; prog[i].func; i++)
		if (!strcmp(progs_to_start[j], prog[i].name))
		{
			printf("Starting program: %s\n"
				 "---------------------------------------\n",
				 prog[i].name);

			if (prog[i].func(NULL))
			{
				printf("\nProgram %s exited with error!\n\n",
					 prog[i].name);
				return -1;
			}
			else {
				printf("\nProgram %s exited successfully!\n\n",
						prog[i].name);
			}
		}

	return 0;
}
