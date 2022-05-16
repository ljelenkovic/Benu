/*! Monitor example (threads) */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

char PROG_HELP[] = "Thread example(monitor): The dining philosophers problem.";

#define PHNUM	5

static timespec_t eat, think;
static int terminate_simulation;

static int stick[PHNUM];
static char phs[PHNUM+1];

static pthread_mutex_t m;
static pthread_cond_t q[PHNUM];

/* philosopher thread */
static void *philosopher(void *param)
{
	int phil, lstick, rstick, lphil, rphil;

	phil = (int) param;
	lstick = phil;
	rstick = (lstick + 1) % PHNUM;
	lphil = (phil + PHNUM - 1) % PHNUM;
	rphil = (phil + 1) % PHNUM;

	printf("%s - Philosopher %d thinking\n", phs, phil);

	while (!terminate_simulation)
	{
		nanosleep(&think, NULL);

		pthread_mutex_lock(&m);
			phs[phil] = '-';
			while (stick[lstick] || stick[rstick])
				pthread_cond_wait(&q[phil], &m);
			stick[lstick] = stick[rstick] = TRUE;
			phs[phil] = 'X';
			printf("%s - Philosopher %d eating\n", phs, phil);
		pthread_mutex_unlock(&m);

		nanosleep(&eat, NULL);

		pthread_mutex_lock(&m);
			stick[lstick] = stick[rstick] = FALSE;
			phs[phil] = 'O';
			printf("%s - Philosopher %d thinking\n", phs, phil);
		pthread_mutex_unlock(&m);

		pthread_cond_signal(&q[lphil]);
		pthread_cond_signal(&q[rphil]);
	}

	return NULL;
}

int monitors(char *args[])
{
	pthread_t thread[PHNUM];
	timespec_t sim_time;
	int i;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	eat.tv_sec = 3;
	eat.tv_nsec = 0;
	think.tv_sec = 3;
	think.tv_nsec = 0;
	sim_time.tv_sec = 30;
	sim_time.tv_nsec = 0;

	terminate_simulation = 0;

	pthread_mutex_init(&m, NULL);

	for (i = 0; i < PHNUM; i++)
	{
		stick[i] = 0;
		phs[i] = 'O';
		pthread_cond_init(&q[i], NULL);
	}
	phs[PHNUM] = '\0';

	for (i = 0; i < PHNUM; i++)
		pthread_create(&thread[i], NULL, philosopher, (void *) i);

	nanosleep(&sim_time, NULL);

	terminate_simulation = 1;

	for (i = 0; i < PHNUM; i++)
		pthread_join(thread[i], NULL);

	pthread_mutex_destroy(&m);

	for (i = 0; i < PHNUM; i++)
		pthread_cond_destroy(&q[i]);

	return 0;
}
