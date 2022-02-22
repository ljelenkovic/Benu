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
	itimerspec_t t1, t2;
	timer_t timer1, timer2;
	sigevent_t evp;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 timer_PROG_HELP);

	clock_gettime(CLOCK_REALTIME, &t);
	t0 = t;
	printf("System time: %d:%d\n", t.tv_sec, t.tv_nsec/100000000);

	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = alarm_nt;
	evp.sigev_notify_attributes = NULL;

	/* timer1 */
	t1.it_interval.tv_sec = 3;
	t1.it_interval.tv_nsec = 0;
	t1.it_value.tv_sec = 3;
	t1.it_value.tv_nsec = 0;
	evp.sigev_value.sival_int = t1.it_interval.tv_sec;
	timer_create(CLOCK_REALTIME, &evp, &timer1);
	timer_settime(&timer1, 0, &t1, NULL);

	/* timer2 */
	t2.it_interval.tv_sec = 5;
	t2.it_interval.tv_nsec = 0;
	t2.it_value.tv_sec = 5;
	t2.it_value.tv_nsec = 0;
	evp.sigev_value.sival_int = t2.it_interval.tv_sec;
	timer_create(CLOCK_REALTIME, &evp, &timer2);
	timer_settime(&timer2, 0, &t2, NULL);

	t.tv_sec = 26;
	t.tv_nsec = 0;

	while (TIME_IS_SET(&t))
		if (clock_nanosleep(CLOCK_REALTIME, 0, &t, &t))
			printf("Interrupted sleep?\n");

	clock_gettime(CLOCK_REALTIME, &t);
	printf("System time: %d:%d\n", t.tv_sec, t.tv_nsec / 100000000);

	timer_delete(&timer1);
	timer_delete(&timer2);

	return 0;
}
