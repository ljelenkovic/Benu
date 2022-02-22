/*! Timer api testing */

#include <stdio.h>
#include <time.h>
#include <api/prog_info.h>
#include <arch/time.h>

static timespec_t t0;

static void alarm()
{
	timespec_t t;

	clock_gettime(CLOCK_REALTIME, &t);
	time_sub(&t, &t0);

	printf("[%d:%d] Alarm! \n", t.tv_sec, t.tv_nsec/100000000);
}

int timer()
{
	timespec_t t;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 timer_PROG_HELP);

	clock_gettime(CLOCK_REALTIME, &t0);
	printf("System time: %d:%d\n", t0.tv_sec, t0.tv_nsec/100000000);

	t.tv_sec = 10;
	t.tv_nsec = 555555555;
	clock_settime(CLOCK_REALTIME, &t);
	printf("System time set to: %d:%d\n", t.tv_sec, t.tv_nsec/100000000);

	clock_gettime(CLOCK_REALTIME, &t0);
	printf("System time: %d:%d\n", t0.tv_sec, t0.tv_nsec/100000000);

	t.tv_sec = 3;
	t.tv_nsec = 0;
	arch_timer_set(&t, alarm);

	do {
		clock_gettime(CLOCK_REALTIME, &t);
	}
	while (t0.tv_sec + 5 > t.tv_sec);

	printf("System time: %d:%d\n", t.tv_sec, t.tv_nsec / 100000000);

	return 0;
}
