/*! Round Robin scheduling test example */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <syscall.h>
#include <arch/processor.h>
#include <kernel/features.h>

char PROG_HELP[] = "Round Robin scheduling example.";

#define THR_NUM	3
#define INNER_LOOP_COUNT 100000
#define TEST_DURATION	10 /* seconds */

static int iters[THR_NUM];
static volatile int end;

/* example threads */
static void *rr_thread(void *param)
{
	int i, j, thr_no;

	thr_no = (int) param;

	printf("RR thread %d starting\n", thr_no);

#if 1 /* testing OS_ENABLE/DISABLE RR */
	if (thr_no == THR_NUM - 1)
	{
		printf("Greedy thread disabling others for some time!\n");
		OS_DISABLE(FEATURE_SCHED_RR);
		for (i = 1; i < 500; i++)
		{
			for (j = 0; j < INNER_LOOP_COUNT && !end; j++)
				memory_barrier();

			iters[thr_no]++;
		}
		OS_ENABLE(FEATURE_SCHED_RR);
		printf("Greedy thread enabling scheduling!\n");
	}
#endif

	for (i = 1; !end; i++)
	{
		for (j = 0; j < INNER_LOOP_COUNT && !end; j++)
			memory_barrier();

		iters[thr_no]++;
	}

	printf("RR thread %d exiting\n", thr_no);

	return NULL;
}

int round_robin(char *args[])
{
	pthread_t thread[THR_NUM];
	pthread_attr_t attr;
	sched_param_t sched_param;
	int i;
	timespec_t sleep;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	sched_param.sched_priority = THREAD_DEF_PRIO/2 + 1;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &sched_param);

	end = FALSE;

	for (i = 0; i < THR_NUM; i++)
	{
		iters[i] = 0;
		pthread_create(&thread[i], &attr, rr_thread, (void *) i);
	}

	printf("Threads created, giving them %d seconds\n", TEST_DURATION);
	sleep.tv_sec = TEST_DURATION;
	sleep.tv_nsec = 0;
	nanosleep(&sleep, NULL);

	printf("Test over - threads are to be canceled\n");
	end = TRUE;

	for (i = 0; i < THR_NUM; i++)
		pthread_join(thread[i], NULL);
	for (i = 0; i < THR_NUM; i++)
		printf("Thread %d, count=%d\n", i, iters[i]);

	return 0;
}
