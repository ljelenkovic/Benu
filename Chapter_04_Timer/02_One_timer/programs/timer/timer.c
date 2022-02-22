/*! Timer api testing */

#include <stdio.h>
#include <time.h>
#include <api/prog_info.h>

static timespec_t t0;

static void alarm_nt(sigval_t param)
{
	int num;
	timespec_t t;

	num = param.sival_int;
	clock_gettime(CLOCK_REALTIME, &t);
	time_sub(&t, &t0);

	printf("[%d:%d] Alarm %d(every %d seconds)\n",
		t.tv_sec, t.tv_nsec/100000000, num, num);
}

int timer()
{
	timespec_t t;
	itimerspec_t it;
	timer_t timer;
	sigevent_t evp;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 timer_PROG_HELP);

	clock_gettime(CLOCK_REALTIME, &t0);
	printf("System time: %d:%d\n", t0.tv_sec, t0.tv_nsec/100000000);

	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = alarm_nt;
	evp.sigev_notify_attributes = NULL;

	/* timer */
	it.it_interval.tv_sec = 3;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = 3;
	it.it_value.tv_nsec = 0;
	evp.sigev_value.sival_int = it.it_interval.tv_sec;
	timer_create(CLOCK_REALTIME, &evp, &timer);
	timer_settime(&timer, 0, &it, NULL);

	do {
		clock_gettime(CLOCK_REALTIME, &t);
	}
	while (t0.tv_sec + 16 > t.tv_sec);

	printf("System time: %d:%d\n", t.tv_sec, t.tv_nsec / 100000000);

	timer_delete(&timer);

	t.tv_sec = 3;
	t.tv_nsec = 0;

	printf("Sleeping %d:%d\n", t.tv_sec, t.tv_nsec / 100000000);

	if (clock_nanosleep(CLOCK_REALTIME, 0, &t, NULL))
		printf("Interrupted sleep?\n");

	clock_gettime(CLOCK_REALTIME, &t);
	printf("System time: %d:%d\n", t.tv_sec, t.tv_nsec / 100000000);

	return 0;
}
