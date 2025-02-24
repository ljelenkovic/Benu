/*! Threads example */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define THR_NUM	3
#define ITERS	5

char PROG_HELP[] = "Thread demonstration example: create several threads that "
		   "perform simple iterations and print basic info.";

static timespec_t sleep;

/* example threads */
static void *simple_thread(void *param)
{
	int i, thr_no;

	thr_no = (int) param;

	printf("Thread %d starting\n", thr_no);
	for (i = 1; i <= ITERS; i++)
	{
		printf("Thread %d: iter %d\n", thr_no, i);
		nanosleep(&sleep, NULL);
	}
	printf("Thread %d exiting\n", thr_no);

	return NULL;
}

int threads(char *args[])
{
	pthread_t thread[THR_NUM];
	int i, j;
	int errno_by_get, errno_by_macro;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	sleep.tv_sec = 1;
	sleep.tv_nsec = 0;

	for (i = 0; i < THR_NUM; i++)
		if (pthread_create(&thread[i], NULL, simple_thread, (void *)i))
		{
			printf("Thread not created!\n");
			break;
		}

	for (j = 0; j < i; j++)
		pthread_join(thread[j], NULL);

	printf("\nerrno test\n");

	errno_by_get = get_errno();
	errno_by_macro = _errno;
	printf("Current errno by get_errno = %d\n", errno_by_get);
	printf("Current errno by _errno = %d\n", errno_by_macro);

	set_errno(10);
	errno_by_get = get_errno();
	errno_by_macro = _errno;
	printf("Current errno by get_errno = %d\n", errno_by_get);
	printf("Current errno by _errno = %d\n", errno_by_macro);

	_errno = 5;
	errno_by_get = get_errno();
	errno_by_macro = _errno;
	printf("Current errno by get_errno = %d\n", errno_by_get);
	printf("Current errno by _errno = %d\n", errno_by_macro);

	return 0;
}
