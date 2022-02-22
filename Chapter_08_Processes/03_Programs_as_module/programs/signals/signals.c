/*! Timer api testing */

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

char PROG_HELP[] = "Signal demonstration.";

static timespec_t t0;

static void sig_handler(siginfo_t *siginfo)
{
	int num;
	timespec_t t, t1;
	int i;

	num = siginfo->si_value.sival_int;
	clock_gettime(CLOCK_REALTIME, &t);
	time_sub(&t, &t0);

	printf("[%d:%d] Signal %d\n",
		t.tv_sec, t.tv_nsec/1000000, num);

	t1.tv_sec = 1;
	t1.tv_nsec = 0;
	for (i = 1; i < 4; i++)
	{
		printf("In signal handler(%d)\n", i);
		clock_nanosleep(CLOCK_REALTIME, 0, &t1, NULL);
	}
}

static void *signal_waiting_thread(void *param)
{
	sigset_t set;
	siginfo_t info;

	sigfillset(&set);
	pthread_sigmask(SIG_BLOCK, &set, NULL);

	printf("Signal waiting thread started\n");
	sigwaitinfo(&set, &info);
	printf("Signal waiting thread got signal:"
		 "num=%d, code=%d, errno=%d, si_value=%d\n",
		 info.si_signo, info.si_code, info.si_errno,
		 info.si_value.sival_int);

	return NULL;
}

int signals(char *args[])
{
	timespec_t t;
	itimerspec_t t1;
	timer_t timer1;
	sigaction_t act;
	sigevent_t evp;
	int i;
	pthread_t thread;
	sigval_t sigval;
	sem_t sem;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	clock_gettime(CLOCK_REALTIME, &t);
	t0 = t;
	printf("[START] System time: %d:%d\n", t.tv_sec, t.tv_nsec/1000000);

	/* signal on timer activation */
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGUSR1;

	act.sa_sigaction = sig_handler;
	act.sa_flags = SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1, &act, NULL);

	/* timer1 */
	t1.it_interval.tv_sec = 6;
	t1.it_interval.tv_nsec = 0;
	t1.it_value.tv_sec = 6;
	t1.it_value.tv_nsec = 0;
	evp.sigev_value.sival_int = SIGUSR1;
	timer_create(CLOCK_REALTIME, &evp, &timer1);
	timer_settime(&timer1, 0, &t1, NULL);

	if (pthread_create(&thread, NULL, signal_waiting_thread, NULL))
		printf("Thread not created!\n");

	t.tv_sec = 1;
	t.tv_nsec = 0;
	sem_init(&sem, 0, 3);
	for (i = 0; i < 10; i++)
	{
		printf("In main thread(%d)\n", i);
		if (i < 5 && sem_wait(&sem) == EXIT_FAILURE)
		{
			int errno = get_errno();
			printf("sem_wait interrupted, errno=%d\n", errno);
			continue;
		}

		if (clock_nanosleep(CLOCK_REALTIME,0,&t,NULL) == EXIT_FAILURE)
		{
			int errno = get_errno();
			printf("Interrupted, errno=%d\n", errno);
		}
	}

	timer_delete(&timer1);

	/* send signal to waiting thread */
	sigval.sival_int = SIGUSR2;

	/* send signal */
	sigqueue(thread, SIGUSR2, sigval);

	pthread_join(thread, NULL);

	clock_gettime(CLOCK_REALTIME, &t);
	printf("[END] System time: %d:%d\n", t.tv_sec, t.tv_nsec / 1000000);

	return 0;
}
