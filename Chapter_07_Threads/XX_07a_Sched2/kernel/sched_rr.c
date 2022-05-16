/*! Round Robin Scheduler */
#define _K_SCHED_RR_C_
#define _K_SCHED_

#include "sched_rr.h"
#include "sched.h"
#include "thread.h"
#include <kernel/errno.h>
#include <kernel/features.h>

static int rr_init(ksched_t *ksched);
static int rr_thread_add(ksched_t *ksched, kthread_t *kthread,
			   int sched_priority, sched_supp_t *sched_param);
static int rr_thread_del(ksched_t *ksched, kthread_t *kthread);
static int rr_thread_activate(ksched_t *ksched, kthread_t *kthread);
static int rr_thread_deactivate(ksched_t *ksched, kthread_t *kthread);
static void rr_timer(sigval_t);

/*! statically defined Round Robin Scheduler */
ksched_t ksched_rr = (ksched_t)
{
	.sched_id =		SCHED_RR,

	.init = 		rr_init,
	.schedule = 		NULL,
	.thread_add =		rr_thread_add,
	.thread_remove =	rr_thread_del,
	.thread_activate =	rr_thread_activate,
	.thread_deactivate =	rr_thread_deactivate,

	.set_thread_sched_parameters =	NULL,
	.get_thread_sched_parameters =	NULL,

	.params.rr.time_slice =	{0, 50000000},
	.params.rr.threshold =	{0, 10000000}
};

/*! Initialize RR scheduler */
static int rr_init(ksched_t *ksched)
{
	sigevent_t evp;

	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = rr_timer;
	evp.sigev_notify_attributes = NULL;
	evp.sigev_value.sival_ptr = ksched;

	ktimer_create(CLOCK_REALTIME, &evp, &ksched->params.rr.ktimer, NULL);
	TIME_RESET(&ksched->params.rr.alarm.it_interval);
	TIME_RESET(&ksched->params.rr.alarm.it_value);

	return 0;
}

/*! Add thread to RR scheduler (give him initial time slice) */
static int rr_thread_add(ksched_t *ksched, kthread_t *kthread,
			   int sched_priority, sched_supp_t *sched_param)
{
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);

	tsched->params.rr.remainder = ksched->params.rr.time_slice;

	return 0;
}

/*! Remove thread from RR scheduler (disarm timer) */
static int rr_thread_del(ksched_t *ksched, kthread_t *kthread)
{
	if (kthread == kthread_get_active(kthread))
	{
		/* disarm timer */
		TIME_RESET(&ksched->params.rr.alarm.it_value);
		ktimer_settime(ksched->params.rr.ktimer, 0,
				 &ksched->params.rr.alarm, NULL);
	}

	return 0;
}


/*! Start time slice for thread (or continue interrupted one) */
static int rr_thread_activate(ksched_t *ksched, kthread_t *kthread)
{
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);

	/* check remainder if needs to be replenished */
	if (time_cmp(&tsched->params.rr.remainder,
		&ksched->params.rr.threshold) <= 0)
	{
		time_add(&tsched->params.rr.remainder,
			   &ksched->params.rr.time_slice);
	}

	/* Get current time and store it */
	kclock_gettime(CLOCK_REALTIME, &tsched->params.rr.slice_start);

	/* When to wake up? */
	tsched->params.rr.slice_end = tsched->params.rr.slice_start;
	time_add(&tsched->params.rr.slice_end, &tsched->params.rr.remainder);

	/* Set alarm for remainder time */
	ksched->params.rr.alarm.it_value = tsched->params.rr.slice_end;

	ktimer_settime(ksched->params.rr.ktimer, TIMER_ABSTIME,
			 &ksched->params.rr.alarm, NULL);

	return 0;
}

/*!
 * Deactivate thread because:
 * 1. higher priority thread becomes active
 * 2. this thread time slice is expired
 * 3. this thread blocks on some queue
 */
static int rr_thread_deactivate(ksched_t *ksched, kthread_t *kthread)
{
	/* Get current time and recalculate remainder */
	timespec_t t;
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);

	if (TIME_IS_SET(&tsched->params.rr.remainder))
	{
		/*
		 * "slice interrupted"
		 * recalculate remainder
		 * remove alarm
		 */
		TIME_RESET(&ksched->params.rr.alarm.it_value);
		ktimer_settime(ksched->params.rr.ktimer, 0,
				 &ksched->params.rr.alarm, NULL);

		kclock_gettime(CLOCK_REALTIME, &t);
		time_sub(&tsched->params.rr.slice_end, &t);
		tsched->params.rr.remainder = tsched->params.rr.slice_end;

		if (kthread_is_active(kthread))
		{
			/* is remainder too small or not? */
			if (time_cmp(&tsched->params.rr.remainder,
				&ksched->params.rr.threshold) <= 0)
			{
				kthread_move_to_ready(kthread, LAST);
			}
			else {
				kthread_move_to_ready(kthread, FIRST);
			}
		}
	}
	/* else = remainder is zero, thread is already enqueued in ready queue*/

	return 0;
}

/*! Timer interrupt for Round Robin */
static void rr_timer(sigval_t sigev_value)
{
	if (!sys__feature(FEATURE_SCHEDULER, FEATURE_GET, 0))
		return; /* scheduler disabled */

	ksched_t *ksched = sigev_value.sival_ptr;
	kthread_t *kthread = kthread_get_active();
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);

	if (ksched != ksched2_get(kthread_get_sched_policy(kthread)))
	{
		/* bug or rr thread got canceled! Let assume second :) */
		LOG(DEBUG, "RR interrupted non RR thread!");
		return;
	}

	/* given time is elapsed, set remainder to zero */
	tsched->params.rr.remainder.tv_sec =
	tsched->params.rr.remainder.tv_nsec = 0;

	/* move thread to ready queue - as last in corresponding queue */
	kthread_move_to_ready(kthread, LAST);

	kthreads_schedule();
}
