/*! Time managing functions */
#define _K_TIME_C_

#include "time.h"

#include "thread.h"
#include "memory.h"
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <arch/time.h>
#include <arch/interrupt.h>
#include <arch/processor.h>
#include <types/bits.h>

static void kclock_wake_thread(sigval_t sigval);
static void kclock_interrupt_sleep(kthread_t *kthread, void *param);
static int ktimer_cmp(void *_a, void *_b);
static void ktimer_schedule();

/*! List of active timers */
static list_t ktimers;

static timespec_t threshold;


/*! Initialize time management subsystem */
int k_time_init()
{
	arch_timer_init();

	/* timer list is empty */
	list_init(&ktimers);

	arch_get_min_interval(&threshold);
	threshold.tv_nsec /= 2;
	if (threshold.tv_sec % 2)
		threshold.tv_nsec += 500000000L; /* + half second */
	threshold.tv_sec /= 2;

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
 * Resume suspended thread (called on timer activation)
 * \param sigval Thread that should be released
 */
static void kclock_wake_thread(sigval_t sigval)
{
	kthread_t *kthread;
	ktimer_t *ktimer;

	kthread = sigval.sival_ptr;
	ASSERT(kthread);

	if (kthread_check_kthread(kthread) &&
		kthread_is_suspended(kthread, NULL, NULL))
	{
		ktimer = kthread_get_private_param(kthread);
		timespec_t *remain = ktimer->param;
		if (remain)
			TIME_RESET(remain); /* timer expired */

		kthread_move_to_ready(kthread, LAST);

		ktimer_delete(ktimer);
	}
	else {
		/*
		 * FIXME I think this shouldn't happen, but am unsure.
		 * Maybe some race condition is still present?
		 */
		//LOG(DEBUG, "Bug maybe!");
	}

	kthreads_schedule();
}

/*! Cancel sleep
 *  - handle return values and errno;
 *  - thread must be handled elsewhere - with source of interrupt (signal?)
 */
static void kclock_interrupt_sleep(kthread_t *kthread, void *param)
{
	ktimer_t *ktimer;
	timespec_t *remain;
	itimerspec_t irem;

	ASSERT(kthread && param);
	ASSERT(kthread_check_kthread(kthread)); /* is this valid thread */
	ASSERT(kthread_is_suspended(kthread, NULL, NULL));

	ktimer = param;
	remain = ktimer->param;

	if (remain)
	{
		/* save remaining time */
		timespec_t now;
		kclock_gettime(CLOCK_REALTIME, &now);
		ktimer_gettime(ktimer, &irem);
		*remain = irem.it_value;
		time_sub(remain, &now);
	}

	ktimer_delete(ktimer);

	kthread_set_syscall_retval(kthread, EXIT_FAILURE);
	kthread_set_errno(kthread, EINTR);
}

/*! Timers ------------------------------------------------------------------ */

/*!
 * Compare timers by expiration times (used when inserting new timer in list)
 * \param a First timer
 * \param b Second timer
 * \return -1 when a < b, 0 when a == b, 1 when a > b
 */
static int ktimer_cmp(void *_a, void *_b)
{
	ktimer_t *a = _a, *b = _b;

	return time_cmp(&a->itimer.it_value, &b->itimer.it_value);
}

/*!
 * Create new timer
 * \param clockid	Clock used in timer
 * \param evp		Timer expiration action
 * \param ktimer	Timer descriptor address is returned here
 * \param owner		Timer owner: thread descriptor or NULL if kernel timer
 * \return status	0 for success
 */
int ktimer_create(clockid_t clockid, sigevent_t *evp, ktimer_t **_ktimer,
		  void *owner)
{
	ktimer_t *ktimer;
	ASSERT(clockid == CLOCK_REALTIME || clockid == CLOCK_MONOTONIC);
	ASSERT(evp && _ktimer);
	/* add other checks on evp if required */

	ktimer = kmalloc(sizeof(ktimer_t));
	ASSERT(ktimer);

	ktimer->id = k_new_id();
	ktimer->clockid = clockid;
	ktimer->evp = *evp;
	ktimer->owner = owner;
	TIMER_DISARM(ktimer);
	ktimer->param = NULL;

	*_ktimer = ktimer;

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

	/* check for object status first */
	if (!k_check_id(ktimer->id))
	{
		/* FIXME Maybe this should not happen! */
		//LOG(DEBUG, "Bug possibly!");
		return ENOENT;
	}

	/* remove from active timers (if it was there) */
	if (TIMER_IS_ARMED(ktimer))
	{
		list_remove(&ktimers, 0, &ktimer->list);
		ktimer_schedule();
	}

	k_free_id(ktimer->id);
	kfree(ktimer);

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

	kclock_gettime(ktimer->clockid, &now);

	if (ovalue)
	{
		*ovalue = ktimer->itimer;

		/* return relative time to timer expiration */
		if (TIME_IS_SET(&ovalue->it_value))
			time_sub(&ovalue->it_value, &now);
	}

	/* first disarm timer, if it was armed */
	if (TIMER_IS_ARMED(ktimer))
	{
		TIMER_DISARM(ktimer);
		list_remove(&ktimers, 0, &ktimer->list);
	}

	if (value && TIME_IS_SET(&value->it_value))
	{
		/* arm timer */
		ktimer->itimer = *value;
		if (!(flags & TIMER_ABSTIME)) /* convert to absolute time */
			time_add(&ktimer->itimer.it_value, &now);

		list_sort_add(&ktimers, ktimer, &ktimer->list, ktimer_cmp);
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

/*! Activate timers and reschedule threads if required */
static void ktimer_schedule()
{
	ktimer_t *first;
	timespec_t time, ref_time;
	int resched = 0;

	if (!k_feature(FEATURE_TIMERS, FEATURE_GET, 0))
		return;

	kclock_gettime(CLOCK_REALTIME, &time);
	/* should have separate "scheduler" for each clock */

	ref_time = time;
	time_add(&ref_time, &threshold);
	/* use "ref_time" instead of "time" when looking timers to activate */

	/* should any timer be activated? */
	first = list_get(&ktimers, FIRST);
	while (first != NULL)
	{
		/* timers have absolute values in 'it_value' */
		if (time_cmp(&first->itimer.it_value, &ref_time) <= 0)
		{
			/* 'activate' timer */

			/* but first remove timer from list */
			first = list_remove(&ktimers, FIRST, NULL);

			/* and add to list if period is given */
			if (TIME_IS_SET(&first->itimer.it_interval))
			{
				/* calculate next activation time */
				time_add(&first->itimer.it_value,
					   &first->itimer.it_interval);
				/* put back into list */
				list_sort_add(&ktimers, first,
						&first->list, ktimer_cmp);
			}
			else {
				TIMER_DISARM(first);
			}

			if (first->owner == NULL)
			{
				/* timer set by kernel - call now, directly */
				if (first->evp.sigev_notify_function)
					first->evp.sigev_notify_function(
						first->evp.sigev_value
					);
			}
			else {
				/* timer set by thread */
				if (!ksignal_process_event(
					&first->evp, first->owner, SI_TIMER))
				{
					resched++;
				}
			}

			first = list_get(&ktimers, FIRST);
		}
		else {
			break;
		}
	}

	first = list_get(&ktimers, FIRST);
	if (first)
	{
		ref_time = first->itimer.it_value;
		time_sub(&ref_time, &time);
		arch_timer_set(&ref_time, ktimer_schedule);
	}

	if (resched)
		kthreads_schedule();
}


/*! Interface to threads ---------------------------------------------------- */

/*!
 * Get current time
 * \param clockid Clock to use
 * \param time Pointer where to store time
 * \return status (0 if successful, -1 otherwise)
 */
int sys__clock_gettime(void *p)
{
	clockid_t clockid;
	timespec_t *time;

	int retval;

	clockid = *((clockid_t *) p);	p += sizeof(clockid_t);
	time = *((void **) p);

	ASSERT_ERRNO_AND_EXIT(
	    time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC),
	    EINVAL
	);
	time =  U2K_GET_ADR(time, kthread_get_process(NULL));
	ASSERT_ERRNO_AND_EXIT(time, EINVAL);

	retval = kclock_gettime(clockid, time);

	EXIT(retval);
}

/*!
 * Set current time
 * \param clockid Clock to use
 * \param time Time to set
 * \return status
 */
int sys__clock_settime(void *p)
{
	clockid_t clockid;
	timespec_t *time;

	int retval;

	clockid = *((clockid_t *) p);	p += sizeof(clockid_t);
	time = *((void **) p);

	ASSERT_ERRNO_AND_EXIT(
	    time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC),
	    EINVAL
	);
	time =  U2K_GET_ADR(time, kthread_get_process(NULL));
	ASSERT_ERRNO_AND_EXIT(time, EINVAL);

	retval = kclock_settime(clockid, time);

	EXIT(retval);
}

/*!
 * Suspend thread until given time elapses
 * \param clockid Clock to use
 * \param flags flags (TIMER_ABSTIME)
 * \param request Suspend duration
 * \param remain Remainder time if interrupted during suspension
 * \return status
 */
int sys__clock_nanosleep(void *p)
{
	clockid_t clockid;
	int flags;
	timespec_t *request;
	timespec_t *remain;

	int retval = EXIT_SUCCESS;
	kthread_t *kthread = kthread_get_active();
	ktimer_t *ktimer;
	sigevent_t evp;
	itimerspec_t itimer;

	clockid =	*((clockid_t *) p);	p += sizeof(clockid_t);
	flags =		*((int *) p);		p += sizeof(int);
	request =	*((timespec_t **) p);	p += sizeof(timespec_t *);
	remain =	*((timespec_t **) p);

	ASSERT_ERRNO_AND_EXIT(
	   (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC) &&
	    request,
	    EINVAL
	);
	request =  U2K_GET_ADR(request, kthread_get_process(NULL));
	ASSERT_ERRNO_AND_EXIT(request, EINVAL);
	ASSERT_ERRNO_AND_EXIT(TIME_IS_SET(request), EINVAL);

	/* Timers are used for "sleep" operations through steps 1-4 */

	/* 1. create timer, but not arm it yet */
	evp.sigev_notify = SIGEV_WAKE_THREAD;
	evp.sigev_value.sival_ptr = kthread;
	evp.sigev_notify_function = kclock_wake_thread;

	retval += ktimer_create(clockid, &evp, &ktimer, kthread);
	ASSERT(retval == EXIT_SUCCESS);

	/* save remainder location, if provided */
	if (remain)
	{
		remain =  U2K_GET_ADR(remain, kthread_get_process(NULL));
		ktimer->param = remain;
	}

	/* 2. suspend thread */
	kthread_set_private_param(kthread, ktimer);
	retval += kthread_suspend(kthread, kclock_interrupt_sleep, ktimer);
	ASSERT(retval == EXIT_SUCCESS);

	/* 3. arm timer */
	TIME_RESET(&itimer.it_interval);
	itimer.it_value = *request;

	retval += ktimer_settime(ktimer, flags, &itimer, NULL);
	ASSERT(retval == EXIT_SUCCESS);

	/* 4. pick other thread as active */
	kthreads_schedule();

	EXIT(retval);
}

/*!
 * Create new timer
 * \param clockid	Clock used in timer
 * \param evp		Timer expiration action
 * \param timerid	Timer descriptor is returned in this variable
 * \return status	0 for success
 */
int sys__timer_create(void *p)
{
	clockid_t clockid;
	sigevent_t *evp;
	timer_t *timerid;

	ktimer_t *ktimer;
	int retval = EXIT_SUCCESS;
	kprocess_t *proc;
	kobject_t *kobj;

	clockid =	*((clockid_t *) p);	p += sizeof(clockid_t);
	evp =		*((sigevent_t **) p);	p += sizeof(sigevent_t *);
	timerid =	*((timer_t **) p);

	proc = kthread_get_process(NULL);
	ASSERT_ERRNO_AND_EXIT(
	clockid == CLOCK_REALTIME || clockid == CLOCK_MONOTONIC, EINVAL);
	ASSERT_ERRNO_AND_EXIT(evp && timerid, EINVAL);
	evp = U2K_GET_ADR(evp, proc);
	timerid = U2K_GET_ADR(timerid, proc);
	ASSERT_ERRNO_AND_EXIT(evp && timerid, EINVAL);

	retval = ktimer_create(clockid, evp, &ktimer, kthread_get_active());
	if (retval == EXIT_SUCCESS)
	{
		kobj = kmalloc_kobject(proc, 0);
		kobj->kobject = ktimer;
		timerid->id = ktimer->id;
		timerid->ptr = kobj;
	}
	EXIT(retval);
}

/*!
 * Delete timer
 * \param timerid	Timer descriptor (user descriptor)
 * \return status	0 for success
 */
int sys__timer_delete(void *p)
{
	timer_t *timerid;

	ktimer_t *ktimer;
	int retval;
	kprocess_t *proc;
	kobject_t *kobj;

	timerid = *((timer_t **) p);

	proc = kthread_get_process(NULL);
	ASSERT_ERRNO_AND_EXIT(timerid, EINVAL);
	timerid = U2K_GET_ADR(timerid, proc);
	ASSERT_ERRNO_AND_EXIT(timerid, EINVAL);
	kobj = timerid->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EINVAL);

	ktimer = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ktimer && ktimer->id == timerid->id, EINVAL);

	retval = ktimer_delete(ktimer);

	kfree_kobject(proc, kobj);

	EXIT(retval);
}

/*!
 * Arm/disarm timer
 * \param timerid	Timer descriptor (user descriptor)
 * \param flags		Various flags
 * \param value		Set timer values (it_value+it_period)
 * \param ovalue	Where to store time to next timer expiration (+period)
 * \return status	0 for success
 */
int sys__timer_settime(void *p)
{
	timer_t *timerid;
	int flags;
	itimerspec_t *value;
	itimerspec_t *ovalue;

	ktimer_t *ktimer;
	int retval;
	kprocess_t *proc;
	kobject_t *kobj;

	timerid = *((timer_t **) p);		p += sizeof(timer_t *);
	flags =	  *((int *) p);		p += sizeof(int);
	value =	  *((itimerspec_t **) p);	p += sizeof(itimerspec_t *);
	ovalue =  *((itimerspec_t **) p);

	proc = kthread_get_process(NULL);
	ASSERT_ERRNO_AND_EXIT(timerid, EINVAL);
	timerid = U2K_GET_ADR(timerid, proc);
	ASSERT_ERRNO_AND_EXIT(timerid, EINVAL);

	kobj = timerid->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EINVAL);

	ktimer = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ktimer && ktimer->id == timerid->id, EINVAL);

	value = U2K_GET_ADR(value, proc);
	ovalue = U2K_GET_ADR(ovalue, proc);

	retval = ktimer_settime(ktimer, flags, value, ovalue);

	EXIT(retval);
}

/*!
 * Get timer expiration time
 * \param timerid	Timer descriptor (user descriptor)
 * \param value		Where to store time to next timer expiration (+period)
 * \return status	0 for success
 */
int sys__timer_gettime(void *p)
{
	timer_t *timerid;
	itimerspec_t *value;

	ktimer_t *ktimer;
	int retval;
	kprocess_t *proc;
	kobject_t *kobj;

	timerid = *((timer_t **) p);		p += sizeof(timer_t *);
	value =	  *((itimerspec_t **) p);

	proc = kthread_get_process(NULL);
	ASSERT_ERRNO_AND_EXIT(timerid, EINVAL);
	timerid = U2K_GET_ADR(timerid, proc);
	ASSERT_ERRNO_AND_EXIT(timerid, EINVAL);

	kobj = timerid->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EINVAL);

	ktimer = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ktimer && ktimer->id == timerid->id, EINVAL);

	value = U2K_GET_ADR(value, proc);

	retval = ktimer_gettime(ktimer, value);

	EXIT(retval);
}
