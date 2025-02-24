/*! Messages */

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <lib/string.h>
#include <errno.h>

char PROG_HELP[] = "Messaging example.";

#define CONSUMERS		2
#define PRODUCERS		3
#define MSGS_PER_PRODUCERS	3
#define MSGS_PER_CONSUMERS	\
((PRODUCERS * MSGS_PER_PRODUCERS + PRODUCERS - 1) / CONSUMERS)

#define MAX_MSG_SIZE	10

static timespec_t sleep = {.tv_sec = 1, .tv_nsec = 0};
static int producers_alive;
static mqd_t mqdes;

/* consumer thread */
static void *consumer(void *param)
{
	int msgs, msgsize, thr_no;
	uint msgprio;
	char msg_buf[MSGS_PER_CONSUMERS*MAX_MSG_SIZE];/* for combined messages*/
	char buffer[MAX_MSG_SIZE];		/* for single message */

	thr_no = (int) param;
	msgs = 0;
	memset(msg_buf, 0, MSGS_PER_CONSUMERS);

	printf("Consumer %d starting\n", thr_no);

	while (1)
	{
		memset(buffer, 0, MAX_MSG_SIZE);
		msgsize = mq_receive(mqdes, buffer, MAX_MSG_SIZE, &msgprio);

		if (msgsize > 0)
		{
			printf("Consumer %d got message: size=%d, msg=%s\n",
				 thr_no, msgsize, buffer);
			strcat(msg_buf, buffer);
			msgs++;
			nanosleep(&sleep, NULL);
		}
		else {
			if (producers_alive > 0)
			{
				printf("Buffer empty, sleeping(%d)!\n",thr_no);
				nanosleep(&sleep, NULL);
			}
			else {
				/* producers done and message queue empty */
				break;
			}
		}
	}
	printf("Consumer %d exiting, received %d messaged(%s)\n",
		 thr_no, msgs, msg_buf);

	return NULL;
}

/* producer thread */
static void *producer(void *param)
{
	int i, thr_no;
	char message[] = "A00";
	char buffer[MAX_MSG_SIZE];

	thr_no = (int) param;
	memset(buffer, 0, MAX_MSG_SIZE);
	buffer[0] = message[0] + thr_no - 1;

	printf("Producer %c starting\n", buffer[0]);

	for (i = 0; i < MSGS_PER_PRODUCERS; i++)
	{
		buffer[1] = message[1] + (i/10) % 10;
		buffer[2] = message[2] + i % 10;

		printf("Producer %c: sends %s\n", buffer[0], buffer);

		while (mq_send(mqdes, buffer, 4, 0))
		{
			printf("Error sending message, retrying(%c)!\n",
				 buffer[0]);
			nanosleep(&sleep, NULL);
		}

		nanosleep(&sleep, NULL);
	}
	printf("Producer %c exiting\n", buffer[0]);
	producers_alive--;

	return NULL;
}

int messages(char *args[])
{
	pthread_t thread[CONSUMERS + PRODUCERS];
	mq_attr_t attr;
	int i;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	attr.mq_flags = 0;
	attr.mq_maxmsg = 3;
	attr.mq_msgsize = MAX_MSG_SIZE;
	attr.mq_curmsgs = 0;

	producers_alive = PRODUCERS;

	mqdes = mq_open("mq1", O_CREAT | O_RDWR | O_NONBLOCK, 0, &attr);
	if (mqdes.id == -1)
	{
		printf("Error creating message queue!\n");
		return EXIT_FAILURE;
	}

	for (i = 0; i < CONSUMERS; i++)
		pthread_create(&thread[i], NULL, consumer, (void *) i+1);
	for (i = 0; i < PRODUCERS; i++)
		pthread_create(&thread[CONSUMERS + i], NULL,
				 producer, (void *) i+1);

	/* wait that all threads completes their work */
	for (i = 0; i < CONSUMERS + PRODUCERS; i++)
		pthread_join(thread[i], NULL);

	mq_close(mqdes);

	return 0;
}
