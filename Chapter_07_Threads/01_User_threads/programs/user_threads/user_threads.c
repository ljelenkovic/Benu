/*! User threads example */

#include "uthread.h"

#include <stdio.h>
#include <errno.h>

char PROG_HELP[] = "Threads created and managed in user space - kernel sees"
		   " only single thread.";

void first (void *param);
void second(void *param);
void third (void *param);

volatile static int thr_num;

int user_threads(char *args[])
{
	int errno_by_get, errno_by_macro;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	uthreads_init();

	(void) create_uthread(first, (void *) 1);
	(void) create_uthread(second, (void *) 2);
	(void) create_uthread(third, (void *) 3);
	thr_num = 3;

	while (	thr_num > 0)
		uthread_yield();

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

/* example threads */
void first(void *param)
{
	int i;

	printf("First thread starting, param %x\n", param);
	for (i = 0; i < 3; i++)
	{
		printf("First thread, iter %d\n", i);
		uthread_yield();
	}
	printf("First thread exiting\n");

	thr_num--;
}

void second(void *param)
{
	int i;

	printf("Second thread starting, param %x\n", param);
	for (i = 0; i < 3; i++)
	{
		printf("Second thread, iter %d\n", i);
		uthread_yield();
	}
	printf("Second thread exiting\n");
	thr_num--;
}

void third(void *param)
{
	int i;

	printf("Third thread starting, param %x\n", param);
	for (i = 0; i < 3; i++)
	{
		printf("Third thread, iter %d\n", i);
		uthread_yield();
	}
	printf("Third thread exiting\n");

	thr_num--;
}
