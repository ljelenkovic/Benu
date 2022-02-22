/*! simple shell interpreter */

#include <stdio.h>
#include <api/prog_info.h>
#include <lib/string.h>
#include <time.h>
#include <kernel/memory.h>
#include <arch/processor.h>

typedef struct _cmd_t_
{
	int (*func)();
	char *name;
	char *descr;
}
cmd_t;

#define MAXCMDLEN	72
#define MAXARGS		10
#define INFO_SIZE	1000

static int help();
static int clear();
static int sysinfo();

static cmd_t sh_cmd[] =
{
	{help, "help", "help - list available commands"},
	{clear, "clear", "clear - clear screen"},
	{sysinfo, "sysinfo", "system information; usage: sysinfo [options]"},
	{NULL, ""}
};

static cmd_t prog[] = PROGRAMS_FOR_SHELL;

int shell()
{
	char cmd[MAXCMDLEN + 1];
	int i, key;
	timespec_t t __attribute__((unused));
	struct pollfd fds = {0 /* stdin */, POLLRDNORM, 0};

	//printf("\x1b[37m"); /* test escape sequence: white text */
	printf("\n*** Simple shell interpreter ***\n\n");
	//printf("\x1b[32m"); /* test escape sequence: green text */
	help();

	t.tv_sec = 0;
	t.tv_nsec = 100000000; /* 100 ms */

	while (1)
	{
		new_cmd:
		printf("\n> ");

		i = 0;
		memset(cmd, 0, MAXCMDLEN);

		/* get command - get chars until new line is received */
		while (i < MAXCMDLEN)
		{
			if (!poll(&fds, 1, 0)) /* is anything pressed? */
			{
				nanosleep(&t, NULL);
				continue;
			}

			key = getchar();
			if (!key) /* not ascii? */
				continue;

			if (key == '\n' || key == '\r')
			{
				if (i > 0)
					break;
				else
					goto new_cmd;
			}

			switch(key)
			{
			case '\b':
				if (i > 0)
				{
					cmd[--i] = 0;
					printf("%c", key);
				}
				break;

			default:
				printf("%c", key);
				cmd[i++] = (char) key;
				break;
			}
		}
		printf("\n");

		/* parse command line */
		for (i = 0; i < MAXCMDLEN; i++)
		{
			if (cmd[i] == 0 || cmd[i] == ' ' || cmd[i] == '\t'
				|| cmd[i] == '\n' || cmd[i] == '\r'
			)
				break;

		}
		cmd[i] = 0;

		/* match command to shell command */
		for (i = 0; sh_cmd[i].func != NULL; i++)
		{
			if (strcmp(cmd, sh_cmd[i].name) == 0)
			{
				if (sh_cmd[i].func())
					printf("\nProgram returned error!\n");

				goto new_cmd;
			}
		}

		/* not shell command; start given program */
		/* match command to program */
		for (i = 0; prog[i].func != NULL; i++)
		{
			if (strcmp(cmd, prog[i].name) == 0)
			{

				if (prog[i].func())
					printf("Program returned error!\n");

				goto new_cmd;
			}
		}

		if (strcmp(cmd, "quit") == 0 ||
			strcmp(cmd, "exit") == 0)
			break;

		/* not program kernel or shell knows about it - report error! */
		printf("Invalid command!");
	}

	printf("Exiting from shell\n");

	return 0;
}

static int help()
{
	int i;

	printf("Shell commands: ");
	for (i = 0; sh_cmd[i].func != NULL; i++)
		printf("%s ", sh_cmd[i].name);
	printf(" quit/exit\n");

	printf("Programs: ");
	for (i = 0; prog[i].func != NULL; i++)
		printf("%s ", prog[i].name);
	printf("\n");

	return 0;
}

static int clear()
{
	printf("\x1b[2J");

	return 0;
}

static int sysinfo()
{
	char info[INFO_SIZE];

	sys__sysinfo(info, INFO_SIZE);

	printf("%s\n", info);

	return 0;
}
