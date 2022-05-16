/*! Time managing functions */
#define _K_TIME_C_

#include "time.h"

#include "memory.h"
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <kernel/features.h>
#include <arch/time.h>
#include <arch/processor.h>

void kclock_wake_up(sigval_t sigval);
void kclock_interrupt_sleep(void *source);
int ktimer_process_event(sigevent_t *evp);
static void ktimer_schedule();

/*! Timer */
static ktimer_t stimer;

static timespec_t threshold;

/* timer for sleep + return value */
static ktimer_t *sleep_timer;
volatile static int sleep_retval, wake_up;


/*! Initialize time management subsystem */
int k_time_init()
{
	arch_timer_init();

	arch_get_min_interval(&threshold);
	threshold.tv_nsec /= 2;
	if (threshold.tv_sec % 2)
		threshold.tv_nsec += 500000000L; /* + half second */
	threshold.tv_sec /= 2;

	TIMER_DISARM(&stimer);

	sleep_timer = NULL;
	sleep_retval = EXIT_SUCCESS;
	wake_up = FALSE;

	return EXIT_SUCCESS;
}

/*!
 * Get current time
 * \param clockid Clock to use
 * \param time Pointer where to store time
 */
int kclock_gettime(clockid_t clockid, timespec_t *time)
{
	ASSERT(time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC));

	arch_get_time(time);

	return EXIT_SUCCESS;
}

/*!
 * Set current time
 * \param clockid Clock to use
 * \param time Time to set
 */
int kclock_settime(clockid_t clockid, timespec_t *time)
{
	ASSERT(time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC));

	arch_set_time(time);

	return EXIT_SUCCESS;
}

/*!
 * Resume suspended program (called on timer activation)
 * \param sigval Not used
 */
void kclock_wake_up(sigval_t sigval)
{
	int retval __attribute__((__unused__));
	timespec_t *remain;

	ASSERT(sleep_timer);
	remain = sleep_timer->param;

	if (remain)
		TIME_RESET(remain); /* timer expired */

	retval = ktimer_delete(sleep_timer);
	ASSERT(retval == EXIT_SUCCESS);

	sleep_timer = NULL;
	sleep_retval = EXIT_SUCCESS;
	set_errno(EXIT_SUCCESS);
	wake_up = TRUE;
}

/*! Cancel sleep
 *  - handle return values and errno;
 */
void kclock_interrupt_sleep(void *source)
{
	int retval;
	timespec_t *remain, now;
	itimerspec_t irem;

	ASSERT(sleep_timer);
	remain = sleep_timer->param;

	if (remain)
	{
		/* save remaining time */
		retval = kclock_gettime(CLOCK_REALTIME, &now);
		retval += ktimer_gettime(sleep_timer, &irem);
		ASSERT(retval == EXIT_SUCCESS);
		*remain = irem.it_value;
		time_sub(remain, &now);
	}

	retval = ktimer_delete(sleep_timer);
	ASSERT(retval == EXIT_SUCCESS);

	sleep_timer = NULL;
	sleep_retval = EXIT_FAILURE;
	set_errno(EINTR);
	wake_up = TRUE;
}

/*! Timers ------------------------------------------------------------------ */

/*!
 * Create new timer
 * \param clockid	Clock used in timer
 * \param evp		Timer expiration action
 * \param ktimer	Timer descriptor address is returned here
 * \return status	0 for success
 */
int ktimer_create(clockid_t clockid, sigevent_t *evp, ktimer_t **ktimer)
{
	ASSERT(clockid == CLOCK_REALTIME || clockid == CLOCK_MONOTONIC);
	ASSERT(evp && ktimer);

	if (TIMER_IS_ARMED(&stimer))
		return EBUSY; /* stimer already in use */

	stimer.clockid = clockid;
	stimer.evp = *evp;
	stimer.param = NULL;

	*ktimer = &stimer;

	return EXIT_SUCCESS;
}

/*!
 * Delete timer
 * \param ktimer	Timer to delete
 * \return status	0 for success
 */
int ktimer_delete(ktimer_t *ktimer)
{
	ASSERT(ktimer);

	if (ktimer != &stimer)
		return EINVAL;

	if (TIMER_IS_ARMED(ktimer))
	{
		TIMER_DISARM(ktimer);
		ktimer_schedule();
	}

	return EXIT_SUCCESS;
}

/*!
 * Arm/disarm timer
 * \param ktimer	Timer
 * \param flags		Various flags
 * \param value		Set timer values (it_value+it_period)
 * \param ovalue	Where to store time to next timer expiration (+period)
 * \return status	0 for success
 */
int ktimer_settime(ktimer_t *ktimer, int flags, itimerspec_t *value,
		     itimerspec_t *ovalue)
{
	timespec_t now;

	ASSERT(ktimer);

	if (ktimer != &stimer)
		return EINVAL;

	kclock_gettime(ktimer->clockid, &now);

	if (ovalue)
	{
		*ovalue = ktimer->itimer;

		/* return relative time to timer expiration */
		if (TIME_IS_SET(&ovalue->it_value))
			time_sub(&ovalue->it_value, &now);
	}

	/* first disarm timer, if it was armed */
	TIMER_DISARM(ktimer);

	if (value && TIME_IS_SET(&value->it_value))
	{
		/* arm timer */
		ktimer->itimer = *value;
		if (!(flags & TIMER_ABSTIME)) /* convert to absolute time */
			time_add(&ktimer->itimer.it_value, &now);
	}

	ktimer_schedule();

	return EXIT_SUCCESS;
}

/*!
 * Get timer expiration time
 * \param ktimer	Timer
 * \param value		Where to store time to next timer expiration (+period)
 * \return status	0 for success
 */
int ktimer_gettime(ktimer_t *ktimer, itimerspec_t *value)
{
	ASSERT(ktimer && value);
	timespec_t now;

	kclock_gettime(ktimer->clockid, &now);

	*value = ktimer->itimer;

	/* return relative time to timer expiration */
	if (TIME_IS_SET(&value->it_value))
		time_sub(&value->it_value, &now);

	return EXIT_SUCCESS;
}

/*! Activate timers */
static void ktimer_schedule()
{
	timespec_t time, ref_time;

	if (!sys__feature(FEATURE_TIMERS, FEATURE_GET, 0))
		return;

	if (!TIME_IS_SET(&stimer.itimer.it_value))
		return;

	kclock_gettime(CLOCK_REALTIME, &time);
	/* should have separate "scheduler" for each clock */

	ref_time = time;
	time_add(&ref_time, &threshold);
	/* use "ref_time" instead of "time" when looking timers to activate */

	/* should timer be activated? */
	/* timer has absolute value in 'it_value' */
	if (time_cmp(&stimer.itimer.it_value, &ref_time) <= 0)
	{
		/* 'activate' timer */

		/* but first, if it is periodic arm for next period */
		if (TIME_IS_SET(&stimer.itimer.it_interval))
		{
			/* calculate next activation time */
			time_add(&stimer.itimer.it_value,
					&stimer.itimer.it_interval);

			ref_time = stimer.itimer.it_value;
			time_sub(&ref_time, &time);
			arch_timer_set(&ref_time, ktimer_schedule);
		}
		else {
			TIMER_DISARM(&stimer);
		}

		ktimer_process_event(&stimer.evp);
	}
	else {
		ref_time = stimer.itimer.it_value;
		time_sub(&ref_time, &time);
		arch_timer_set(&ref_time, ktimer_schedule);
	}
}

/*! Process event defined with sigevent_t */
int ktimer_process_event(sigevent_t *evp)
{
	int retval = EXIT_SUCCESS;
	void (*func)(sigval_t);

	ASSERT(evp);

	switch(evp->sigev_notify)
	{
	case SIGEV_WAKE_THREAD:	/* for *sleep */
	case SIGEV_THREAD:	/* no threads yet, just call given function */
		func = evp->sigev_notify_function;
		func(evp->sigev_value);
		break;

	case SIGEV_NONE:
		break;

	case SIGEV_THREAD_ID:
	case SIGEV_SIGNAL:
		retval = ENOTSUP;

		break;

	default:
		retval = EINVAL;
		break;
	}

	return retval;
}


/*! Interface to programs --------------------------------------------------- */

/*!
 * Get current time
 * \param clockid Clock to use
 * \param time Pointer where to store time
 * \return status (0 if successful, -1 otherwise)
 */
int sys__clock_gettime(clockid_t clockid, timespec_t *time)
{
	int retval;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(
		time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC),
		EINVAL
	);

	retval = kclock_gettime(clockid, time);

	SYS_EXIT(retval, retval);
}

/*!
 * Set current time
 * \param clockid Clock to use
 * \param time Time to set
 * \return status
 */
int sys__clock_settime(clockid_t clockid, timespec_t *time)
{
	int retval;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(
		time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC),
		EINVAL
	);

	retval = kclock_settime(clockid, time);

	SYS_EXIT(retval, retval);
}


/*!
 * Suspend program until given time elapses
 * \param clockid Clock to use
 * \param flags flags (TIMER_ABSTIME)
 * \param request Suspend duration
 * \param remain Remainder time if interrupted during suspension
 * \return status
 */
int sys__clock_nanosleep(clockid_t clockid, int flags,
			   timespec_t *request, timespec_t *remain)
{
	int retval;
	sigevent_t evp;
	itimerspec_t itimer;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(
	   (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC) &&
	    request && TIME_IS_SET(request),
	    EINVAL
	);

	/* Timers are used for "sleep" operations through steps 1-4 */

	/* 1. create timer */
	evp.sigev_notify = SIGEV_WAKE_THREAD;
	evp.sigev_value.sival_ptr = NULL;
	evp.sigev_notify_function = kclock_wake_up;

	retval = ktimer_create(clockid, &evp, &sleep_timer);
	ASSERT_ERRNO_AND_EXIT(retval == EXIT_SUCCESS, EBUSY);
	sleep_timer->param = remain;

	/* 2. arm timer */
	TIME_RESET(&itimer.it_interval);
	itimer.it_value = *request;

	retval += ktimer_settime(sleep_timer, flags, &itimer, NULL);
	ASSERT(retval == EXIT_SUCCESS);

	set_errno(EXIT_SUCCESS);

	/* 3. wait for timer activation */
	sleep_retval = EXIT_SUCCESS;
	wake_up = FALSE;

	do {
		enable_interrupts();
		suspend();		/* suspend till next interrupt */
		disable_interrupts();
	}
	while (wake_up == FALSE);

	SYS_EXIT(get_errno(), sleep_retval);
}

/*!
 * Create new timer
 * \param clockid	Clock used in timer
 * \param evp		Timer expiration action
 * \param timerid	Timer descriptor is returned in this variable
 * \return status	0 for success
 */
int sys__timer_create(clockid_t clockid, sigevent_t *evp, timer_t *timerid)
{
	ktimer_t *ktimer;
	int retval;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(
	clockid == CLOCK_REALTIME || clockid == CLOCK_MONOTONIC, EINVAL);
	ASSERT_ERRNO_AND_EXIT(evp && timerid, EINVAL);

	retval = ktimer_create(clockid, evp, &ktimer);
	if (retval == EXIT_SUCCESS)
	{
		kobj = kmalloc_kobject(0);
		kobj->kobject = ktimer;
		timerid->id = ktimer->id;
		timerid->ptr = kobj;
	}

	SYS_EXIT(retval, retval);
}

/*!
 * Delete timer
 * \param timerid	Timer descriptor (user descriptor)
 * \return status	0 for success
 */
int sys__timer_delete(timer_t *timerid)
{
	ktimer_t *ktimer;
	int retval;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(timerid, EINVAL);
	kobj = timerid->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);

	ktimer = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ktimer && ktimer->id == timerid->id, EINVAL);

	retval = ktimer_delete(ktimer);

	kfree_kobject(kobj);

	SYS_EXIT(retval, retval);
}

/*!
 * Arm/disarm timer
 * \param timerid	Timer descriptor (user descriptor)
 * \param flags		Various flags
 * \param value		Set timer values (it_value+it_period)
 * \param ovalue	Where to store time to next timer expiration (+period)
 * \return status	0 for success
 */
int sys__timer_settime(timer_t *timerid, int flags,
			 itimerspec_t *value, itimerspec_t *ovalue)
{
	ktimer_t *ktimer;
	int retval;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(timerid, EINVAL);
	kobj = timerid->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);

	ktimer = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ktimer && ktimer->id == timerid->id, EINVAL);

	retval = ktimer_settime(ktimer, flags, value, ovalue);

	SYS_EXIT(retval, retval);
}

/*!
 * Get timer expiration time
 * \param timerid	Timer descriptor (user descriptor)
 * \param value		Where to store time to next timer expiration (+period)
 * \return status	0 for success
 */
int sys__timer_gettime(timer_t *timerid, itimerspec_t *value)
{
	ktimer_t *ktimer;
	int retval;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(timerid, EINVAL);
	kobj = timerid->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);

	ktimer = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ktimer && ktimer->id == timerid->id, EINVAL);

	retval = ktimer_gettime(ktimer, value);

	SYS_EXIT(retval, retval);
}
