/*! EDF scheduling test example */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <arch/processor.h>

char PROG_HELP[] = "EDF scheduling demonstration example.";
char EXTRA_INFO[] = "\n(manual loop calibration requied for optimal display of"
" EDF\n loop last differently in debug than in optimized version)\n\n";

#define THR_NUM	4
#define TEST_DURATION	20 /* seconds */

#define LOOPS	60000000 /* adjust manually per processor to be ~0,3 s */

static timespec_t t0;
static volatile int end;

void message(int thread, char *action)
{
	timespec_t t;

	clock_gettime(CLOCK_REALTIME, &t);
	time_sub(&t, &t0);
	printf("[%d:%d] Thread %d -- %s\n",
		t.tv_sec, t.tv_nsec/100000000, thread, action);
}

/* EDF thread */
static void *edf_thread(void *param)
{
	int thr_no, i, j;
	thr_no = (int) param;
	timespec_t period, deadline;

	i = thr_no;
	period.tv_sec = thr_no * 1;
	period.tv_nsec = 0;
	deadline.tv_sec = thr_no / 2;
	deadline.tv_nsec = (thr_no % 2) * 500000000;

	message(thr_no, "EDF_SET");
	edf_set(deadline, period, EDF_TERMINATE);

	for (i = 0; !end; i++)
	{
		message(thr_no, "EDF_WAIT");
		if (edf_wait())
		{
			message(thr_no, "Deadline missed, exiting!");
			break;
		}

		message(thr_no, "run");
		for (j = 1; j <= LOOPS; j++)
			memory_barrier();
	}

	message(thr_no, "EDF_EXIT");
	edf_exit();

	return NULL;
}

/* unimportant thread */
static void *unimportant_thread(void *param)
{
	timespec_t sleep;

	sleep.tv_sec = 0;
	sleep.tv_nsec = 100000000;

	while (!end)
	{
		message(0, "unimportant thread");
		nanosleep(&sleep, NULL);
	}

	return NULL;
}

int edf(char *args[])
{
	pthread_t thread[THR_NUM + 1];
	pthread_attr_t attr;
	sched_param_t sched_param;
	int i;
	timespec_t sleep;

	printf("Example program: [%s:%s]\n%s\n", __FILE__, __FUNCTION__,
		 PROG_HELP);
	printf(EXTRA_INFO);

	end = FALSE;

	clock_gettime(CLOCK_REALTIME, &t0);

	for (i = 0; i < THR_NUM; i++)
		pthread_create(&thread[i], NULL, edf_thread, (void *)(i+1));

	sched_param.sched_priority = THREAD_DEF_PRIO/2 + 1;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	pthread_attr_setschedparam(&attr, &sched_param);

	pthread_create(&thread[i], &attr, unimportant_thread, (void *)(i+1));

	printf("Threads created, giving them %d seconds\n", TEST_DURATION);

	sleep.tv_sec = TEST_DURATION;
	sleep.tv_nsec = 0;
	nanosleep(&sleep, NULL);

	printf("Test over - threads are to be canceled\n");

	end = TRUE;

	for (i = 0; i < THR_NUM + 1; i++)
		pthread_join(thread[i], NULL);

	return 0;
}
