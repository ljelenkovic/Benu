/*! run all programs */

#include <stdio.h>
#include <lib/string.h>
#include <time.h>
#include <syscall.h>
#include <pthread.h>

char PROG_HELP[] = "Run all compiled programs";

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
static int power_off ( char *args[] );
static int set ( char *args[] );

static cmd_t sh_cmd[] =
{
	{ power_off, "poweroff", "poweroff - use ACPI to power off" },
	{ NULL, "" }
};

int run_all ( char *args[] )
{
	pthread_t thr;
	int *status;
	char info[INFO_SIZE];
	char *args[] = { "sysinfo", "programs", NULL };
	char *progname;

	printf ( "\n*** Running all programs, one by one ***\n\n" );

	syscall ( SYSINFO, &info, INFO_SIZE, args );
	printf ( "%s\n", info );


	while (1)
	{

		/* not shell command; start given program by calling kernel */
		rv = posix_spawn ( &thr, argval[0], NULL, NULL, NULL, NULL );
		if ( !rv )
		{
			if ( pthread_join ( thr, &status ) )
			{
				printf ( "pthread_join error!\n" );
				return 1;
			}

			if ( !status || !(*status) )
			{
				printf("Program %s exited with error!\n", name);
				return 1;
			}
		}

		/* not program kernel or shell knows about it - report error! */
		printf ( "Invalid command!" );
	}

	printf ( "All programs completed succesfuly!\n" );

	return 0;
}

static int power_off ( char *args[] )
{
	printf ( "Powering off\n\n" );
	syscall ( POWER_OFF, NULL, 0, NULL );

	return -1;
}

