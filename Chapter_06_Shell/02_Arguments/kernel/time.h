/*! Time managing functions */
#pragma once

#include <kernel/time.h>

/*! interface to kernel */

#ifndef _K_TIME_C_
typedef void ktimer_t;
#else
struct _ktimer_t_; typedef struct _ktimer_t_ ktimer_t;
#endif /* _K_TIME_C_ */

int k_time_init();
int kclock_gettime(clockid_t clockid, timespec_t *time);
int kclock_settime(clockid_t clockid, timespec_t *time);

int ktimer_create(clockid_t clockid, sigevent_t *evp, ktimer_t **_ktimer);
int ktimer_delete(ktimer_t *ktimer);
int ktimer_settime(ktimer_t *ktimer, int flags, itimerspec_t *value,
		   itimerspec_t *ovalue);
int ktimer_gettime(ktimer_t *ktimer, itimerspec_t *value);

/* signal notification type for wakeup */
#define	SIGEV_WAKE_THREAD	(SIGEV_THREAD_ID + 1)


#ifdef	_K_TIME_C_
/*! rest of the file is only for 'kernel/timer.c' --------------------------- */

#include <lib/list.h>

/*! Kernel timer */
struct _ktimer_t_
{
	id_t	      id;
		      /* timer id */

	clockid_t     clockid;
		      /* which clock to use */
	sigevent_t    evp;
		      /* what to do when timer expires */
	itimerspec_t  itimer;
		      /* interval timers {it_value, it_interval} */

	void	     *param;
		      /* additional parameter (remainder for sleep)*/

	list_h	      list;
		      /* active timers are in sorted list */
};

#define TIMER_IS_ARMED(T)	TIME_IS_SET(&(T)->itimer.it_value)
#define TIMER_DISARM(T)		TIME_RESET(&(T)->itimer.it_value)

#endif	/* _K_TIME_C_ */
