/*! run all programs */

#include <stdio.h>
#include <lib/string.h>
#include <time.h>
#include <syscall.h>
#include <pthread.h>

char PROG_HELP[] = "Run all compiled programs";

#define INFO_SIZE	1000

static int power_off ( char *args[] );

int run_all ( char *args[] )
{
	pthread_t thr;
	int *status, rv;
	char *progname;

#if 0	/* run all programs */

	char info[INFO_SIZE];
	char *sysinfo_args[] = { "sysinfo", "programs", NULL };

	printf ( "\n*** Running all programs, one by one ***\n\n" );

	syscall ( SYSINFO, &info, INFO_SIZE, sysinfo_args );
	printf ( "%s\n", info );

	progname = strstr ( info, ":\n" );

#else /* run selected programs */

	char progs_to_start[] = {
		"hello timer args uthreads threads semaphores "
		"monitors messages signals rr edf" };
	progname = progs_to_start;

#endif

	/* progname has program names to be started */
	progname = strtok ( progname, ":\n " );
	while ( progname )
	{
		printf ( "Starting program: %s\n"
			 "---------------------------------------\n", progname );
#if 1
		rv = posix_spawn ( &thr, progname, NULL, NULL, NULL, NULL );
		if ( !rv )
		{
			if ( pthread_join ( thr, (void **) &status ) )
			{
				printf ( "\npthread_join error!\n\n" );
				break;
			}

			if ( status && !(*status) )
			{
				printf ( "\nProgram %s exited with error!\n\n",
					 progname );
				break;
			}
			else {
				printf ( "\nProgram %s exited successfully!\n\n",
					 progname );
			}
		}
		else {
			printf ( "\nProgram: %s not started!\n", progname );
		}
#endif
		progname = strtok ( NULL, " " );
	}

	power_off (NULL);

	return 0;
}

static int power_off ( char *args[] )
{
	printf ( "Powering off\n\n" );
	syscall ( POWER_OFF, NULL, 0, NULL );

	return -1;
}

