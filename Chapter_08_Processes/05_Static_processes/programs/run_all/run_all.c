/*! run selected programs and poweroff */

#include <stdio.h>
#include <lib/string.h>
#include <time.h>
#include <syscall.h>
#include <pthread.h>
#include <errno.h>

char PROG_HELP[] = "Run all compiled programs";

#define INFO_SIZE	1000

int run_all(char *args[])
{
	pthread_t thr;
	int *status, rv;
	char *progname;

#if 0	/* run all programs */

	char info[INFO_SIZE];
	char *sysinfo_args[] = {"sysinfo", "programs", NULL};

	printf("\n*** Running all programs, one by one ***\n\n");

	syscall(SYSINFO, &info, INFO_SIZE, sysinfo_args);
	printf("%s\n", info);

	progname = strstr(info, ":\n");

#else /* run selected programs */

	char progs_to_start[] = {
		"hello timer args uthreads threads semaphores "
		"monitors messages signals rr" };
	progname = progs_to_start;

#endif

	/* progname has program names to be started */
	progname = strtok(progname, ":\n ");
	while (progname)
	{
		printf("Starting program: %s\n"
			 "---------------------------------------\n", progname);

		rv = posix_spawn(&thr, progname, NULL, NULL, NULL, NULL);
		if (!rv)
		{
			rv = pthread_join(thr, (void **) &status);
			if (rv && get_errno() != ESRCH)
			{
				printf("\npthread_join error!\n\n");
				break;
			}
#if 0
			/* most functions do not call pthread_exit directly so
			 * the following test might be "false negative" */
			else if (status && *status)
			{
				printf("\nProgram %s exited with error!\n\n",
					 progname);
				break;
			}
#endif
			else {
				printf("\nProgram %s exited successfully!\n\n",
					progname);
			}
		}
		else {
			printf("\nProgram: %s not started!\n", progname);
			break;
		}

		progname = strtok(NULL, " ");
	}

	return 0;
}
