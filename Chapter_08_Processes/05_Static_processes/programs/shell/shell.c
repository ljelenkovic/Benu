/*! simple shell interpreter */

#include <stdio.h>
#include <lib/string.h>
#include <time.h>
#include <syscall.h>
#include <pthread.h>

char PROG_HELP[] = "Simple command shell";

typedef struct _cmd_t_
{
	int (*func) ( char *argv[] );
	char *name;
	char *descr;
}
cmd_t;

#define MAXCMDLEN	72
#define MAXARGS		10
#define PROG_LIST_SIZE	1000
#define INFO_SIZE	1000

static char s_stdout[MAXCMDLEN];
static char s_stdin[MAXCMDLEN];

static int help ();
static int clear ();
static int sysinfo ( char *args[] );
static int set ( char *args[] );

static cmd_t sh_cmd[] =
{
	{ help, "help", "help - list available commands" },
	{ clear, "clear", "clear - clear screen" },
	{ sysinfo, "sysinfo", "system information; usage: sysinfo [options]" },
	{ set, "set", "change shell settings; "
		"usage: set stdin|stdout [device]" },
	{ NULL, "" }
};

int shell ( char *args[] )
{
	char cmd[MAXCMDLEN + 1];
	int i, key, rv;
	timespec_t t;
	int argnum;
	char *argval[MAXARGS + 1];
	pthread_t thr;

	printf ( "\n*** Simple shell interpreter ***\n\n" );
	help ();

	t.tv_sec = 0;
	t.tv_nsec = 100000000; /* 100 ms */

	strcpy ( s_stdout, U_STDOUT );
	strcpy ( s_stdin, U_STDIN );

	while (1)
	{
		new_cmd:
		printf ( "\n> " );

		i = 0;
		memset ( cmd, 0, MAXCMDLEN );

		/* get command - get chars until new line is received */
		while ( i < MAXCMDLEN )
		{
			key = get_char ();

			if ( !key )
			{
				nanosleep ( &t, NULL );
				continue;
			}

			if ( key == '\n' || key == '\r')
			{
				if ( i > 0 )
					break;
				else
					goto new_cmd;
			}

			switch ( key )
			{
			case '\b':
				if ( i > 0 )
				{
					cmd[--i] = 0;
					printf ( "%c", key );
				}
				break;

			default:
				printf ( "%c", key );
				cmd[i++] = (char) key;
				break;
			}
		}
		printf ( "\n" );

		/* parse command line */
		argnum = 0;
		for(i = 0; i < MAXCMDLEN && cmd[i]!=0 && argnum < MAXARGS; i++)
		{
			if ( cmd[i] == ' ' || cmd[i] == '\t')
				continue;

			argval[argnum++] = &cmd[i];
			while ( cmd[i] && cmd[i] != ' ' && cmd[i] != '\t'
				&& i < MAXCMDLEN )
				i++;

			cmd[i] = 0;
		}
		argval[argnum] = NULL;

		/* match command to shell command */
		for ( i = 0; sh_cmd[i].func != NULL; i++ )
		{
			if ( strcmp ( argval[0], sh_cmd[i].name ) == 0 )
			{
				if ( sh_cmd[i].func ( argval ) )
					printf ( "\nProgram returned error!\n" );

				goto new_cmd;
			}
		}

		/* not shell command; start given program by calling kernel */
		rv = posix_spawn ( &thr, argval[0], NULL, NULL, argval, NULL );
		if ( !rv )
		{
			if ( argnum < 2 || argval[argnum-1][0] != '&' )
				pthread_join ( thr, NULL );

			goto new_cmd;
		}

		if ( strcmp ( argval[0], "quit" ) == 0 ||
			strcmp ( argval[0], "exit" ) == 0 )
			break;

		/* not program kernel or shell knows about it - report error! */
		printf ( "Invalid command or program already started!" );
	}

	printf ( "Exiting from shell\n" );

	return 0;
}

static int help ()
{
	int i;

	printf ( "Shell commands: " );
	for ( i = 0; sh_cmd[i].func != NULL; i++ )
		printf ( "%s ", sh_cmd[i].name );
	printf ( " quit/exit\n" );

	return 0;
}

static int clear ()
{
	return clear_screen ();
}

static int sysinfo ( char *args[] )
{
	char info[INFO_SIZE];

	syscall ( SYSINFO, &info, INFO_SIZE, args );

	printf ( "%s\n", info );

	return 0;
}

static int set ( char *args[] )
{
	if ( args[1] == NULL ||
		( strcmp ( args[1], "stdin"  ) &&
		  strcmp ( args[1], "stdout" )		)	)
	{
		printf ( "'set' usage: set stdin|stdout [device]\n" );
	}
	else if ( args[2] == NULL )
	{
		/* print current configuration */
		printf ( "console: stdout = %s, stdin = %s\n",
			s_stdout, s_stdin );
	}
	else if ( strcmp ( args[1], "stdin" ) == 0 )
	{
		if ( strcmp ( args[2], s_stdin ) == 0 )
		{
			printf ( "Given stdin (%s) is already in use!\n",
				 args[2] );
		}
		else if ( change_stdin ( args[2] ) )
		{
			printf ( "Error in changing stdin to %s\n", args[2] );
		}
		else {
			printf ("stdin changed to %s (for console)\n", args[2]);
			printf ("(stdin for programs is still %s)\n", U_STDIN );

			strcpy ( s_stdin, args[2] );
		}
	}
	else if ( strcmp ( args[1], "stdout" ) == 0 )
	{
		if ( strcmp ( args[2], s_stdout ) == 0 )
		{
			printf ( "Given stdout (%s) is already in use!\n",
				 args[2] );
		}
		else if ( change_stdout ( args[2] ) )
		{
			printf ( "Error in changing stdout to %s\n", args[2] );
		}
		else {
			printf("stdout changed to %s (for console)\n", args[2]);
			printf("(stdout for programs is still %s)\n", U_STDOUT);

			strcpy ( s_stdout, args[2] );
		}
	}
	else {
		/* must not get here */
		printf ( "Internal shell error!\n" );
	}

	return 0;
}