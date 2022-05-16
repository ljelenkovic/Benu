/*! Time types and constants */
#pragma once

#include <types/basic.h>

/* time formats */

typedef long time_t;		/* Used for time in seconds */

typedef struct timespec {
	time_t  tv_sec;		/* seconds */
	long    tv_nsec;	/* nanoseconds */
}
timespec_t;

typedef uint clockid_t;
/* Used for clock ID type in the clock and timer functions */

#define CLOCK_REALTIME	1
#define CLOCK_MONOTONIC	2

#define TIME_IS_SET(T)	((T)->tv_sec + (T)->tv_nsec != 0)
#define TIME_RESET(T)	do {(T)->tv_sec = (T)->tv_nsec = 0; } while (0)


/*! Short functions - timespec_t manipulation -------------------------- */

/*!
 * Calculate a = a + b (both represent time)
 * \param a First time
 * \param b Second time
 */
static inline void time_add(timespec_t *a, timespec_t *b)
{
	a->tv_sec = a->tv_sec + b->tv_sec;
	a->tv_nsec = a->tv_nsec + b->tv_nsec;

	if (a->tv_nsec > 1000000000L)
	{
		a->tv_sec++;
		a->tv_nsec -= 1000000000L;
	}
}

/*!
 * Calculate a = a - b (both represent time)
 * \param a First time
 * \param b Second time
 * NOTE: assumes a > b, does not check this!
 */
static inline void time_sub(timespec_t *a, timespec_t *b)
{
	if (a->tv_nsec >= b->tv_nsec)
	{
		a->tv_sec -= b->tv_sec;
		a->tv_nsec -= b->tv_nsec;
	}
	else {
		a->tv_sec -= b->tv_sec + 1;
		a->tv_nsec += 1000000000L - b->tv_nsec;
	}
}

/*!
 * Compare times
 * \param a First time
 * \param b Second time
 * \return -1 when a < b, 0 when a == b, 1 when a > b
 */
static inline int time_cmp(timespec_t *a, timespec_t *b)
{
	if (a->tv_sec < b->tv_sec)
		return -1;

	if (a->tv_sec > b->tv_sec)
		return 1;

	/* a->tv_sec == b->tv_sec */

	if (a->tv_nsec < b->tv_nsec)
		return -1;

	if (a->tv_nsec > b->tv_nsec)
		return 1;

	/* a->tv_nsec == b->tv_nsec */
	return 0;
}
