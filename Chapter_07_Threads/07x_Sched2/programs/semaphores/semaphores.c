/*! Semaphore example (threads) */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

char PROG_HELP[] = "Thread example(semaphore): producer consumer example.";

#define CONSUMERS	2
#define PRODUCERS	3
#define BUFF_SIZE	5

static timespec_t sleep;
static int end_msgs;

static sem_t filled, empty, crit1, crit2;

static int buffer[BUFF_SIZE], in, out;

/* consumer thread */
static void *consumer(void *param)
{
	int i, thr_no, mybuff[BUFF_SIZE*PRODUCERS], items;

	thr_no = (int) param;
	items = 0;

	printf("Consumer %d starting\n", thr_no);

	while (end_msgs < PRODUCERS)
	{
		sem_wait(&filled);
		sem_wait(&crit1);

		if (end_msgs >= PRODUCERS)
		{
			sem_post(&crit1);
			sem_post(&empty);
			break;
		}

		mybuff[items] = buffer[out];
		if (++out >= BUFF_SIZE)
			out %= BUFF_SIZE;

		if (!mybuff[items])
			end_msgs++;

		printf("Consumer %d: got %d\n", thr_no, mybuff[items++]);

		sem_post(&crit1);
		sem_post(&empty);

		nanosleep(&sleep, NULL);
	}

	printf("Consumer %d exiting, received: ", thr_no);
	for (i = 0; i < items; i++)
		printf("%d ", mybuff[i]);
	printf("\n");

	sem_post(&filled); /* release 'fellow' consumer since no more data */

	return NULL;
}

/* producer thread */
static void *producer(void *param)
{
	int i, thr_no, data;

	thr_no = (int) param;

	printf("Producer %d starting\n", thr_no);

	for (i = 1; i < BUFF_SIZE; i++)
	{
		sem_wait(&empty);
		sem_wait(&crit2);

		data = (i < BUFF_SIZE - 1 ? thr_no * 10 + i % 10 : 0);

		printf("Producer %d: sends %d\n", thr_no, data);

		buffer[in] = data;
		if (++in >= BUFF_SIZE)
			in %= BUFF_SIZE;

		sem_post(&crit2);
		sem_post(&filled);

		nanosleep(&sleep, NULL);
	}
	printf("Producer %d exiting\n", thr_no);

	return NULL;
}

int semaphores(char *args[])
{
	pthread_t thread[CONSUMERS + PRODUCERS];
	int i;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	sleep.tv_sec = 1;
	sleep.tv_nsec = 0;

	sem_init(&filled, FALSE, 0);
	sem_init(&empty, FALSE, BUFF_SIZE);
	sem_init(&crit1, FALSE, 1);
	sem_init(&crit2, FALSE, 1);
	in = out = 0;
	end_msgs = 0;

	for (i = 0; i < CONSUMERS; i++)
		pthread_create(&thread[i], NULL, consumer, (void *)(i+1));

	for (i = 0; i < PRODUCERS; i++)
		pthread_create(&thread[CONSUMERS+i], NULL,
				 producer, (void *)(i+1));

	for (i = 0; i < CONSUMERS + PRODUCERS; i++)
		pthread_join(thread[i], NULL);

	sem_destroy(&filled);
	sem_destroy(&empty);
	sem_destroy(&crit1);
	sem_destroy(&crit2);

	return 0;
}
