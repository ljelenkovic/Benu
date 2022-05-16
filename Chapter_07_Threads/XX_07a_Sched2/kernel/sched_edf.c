/*! EDF Scheduler */
#define _K_SCHED_EDF_C_
#define _K_SCHED_

#include "sched.h"
#include "time.h"
#include <kernel/errno.h>
#include <types/basic.h>

static int edf_init(ksched_t *ksched);
static int edf_thread_add(ksched_t *ksched, kthread_t *kthread,
			     int sched_priority, sched_supp_t *sched_param);
static int edf_thread_remove(ksched_t *ksched, kthread_t *kthread);
static int edf_set_thread_sched_parameters(ksched_t *ksched,
					     kthread_t *kthread,
					     sched_supp_t *params);
static int edf_thread_deactivate(ksched_t *ksched, kthread_t *kthread);

static int edf_schedule(ksched_t *ksched);

static void edf_period_alarm(sigval_t sigev_value);
static void edf_deadline_alarm(sigval_t sigev_value);

static int edf_check_deadline(kthread_t *kthread);

/*! statically defined Earliest-Deadline-First Scheduler */
ksched_t ksched_edf = (ksched_t)
{
	.sched_id =			SCHED_EDF,

	.init = 			edf_init,
	.schedule = 			edf_schedule,
	.thread_add =			edf_thread_add,
	.thread_remove =		edf_thread_remove,
	.thread_activate =		NULL,
	.thread_deactivate =		edf_thread_deactivate,
	.set_thread_sched_parameters =	edf_set_thread_sched_parameters,
	.get_thread_sched_parameters =	NULL,

	.params.edf.active =		NULL
};

/*! Initialize EDF scheduler */
static int edf_init(ksched_t *ksched)
{
	ksched->params.edf.active = NULL;
	kthreadq_init(&ksched->params.edf.ready);
	kthreadq_init(&ksched->params.edf.wait);

	return 0;
}
static int edf_thread_add(ksched_t *ksched, kthread_t *kthread,
			     int sched_priority, sched_supp_t *sched_param)
{
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);

	tsched->params.edf.period_alarm = NULL;
	tsched->params.edf.deadline_alarm = NULL;

	if (sched_param && sched_param->edf.flags & EDF_SET)
		edf_set_thread_sched_parameters(ksched, kthread, sched_param);

	return 0;
}

static int edf_thread_remove(ksched_t *ksched, kthread_t *kthread)
{
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);

	if (ksched->params.edf.active == kthread)
		ksched->params.edf.active = NULL;

	if (tsched->params.edf.period_alarm)
	{
		ktimer_delete(tsched->params.edf.period_alarm);
		tsched->params.edf.period_alarm = NULL;
	}
	if (tsched->params.edf.deadline_alarm)
	{
		ktimer_delete(tsched->params.edf.deadline_alarm);
		tsched->params.edf.deadline_alarm = NULL;
	}

	edf_schedule(ksched);

	return 0;
}

static int edf_create_alarm(kthread_t *kthread, void *action, void **timer)
{
	sigevent_t evp;

	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = action;
	evp.sigev_notify_attributes = NULL;
	evp.sigev_value.sival_ptr = kthread;

	return ktimer_create(CLOCK_REALTIME, &evp, timer, NULL);
}

static int edf_set_thread_sched_parameters(ksched_t *ksched,
					     kthread_t *kthread,
					     sched_supp_t *params)
{
	timespec_t now;
	itimerspec_t alarm;
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);

	if (ksched->params.edf.active == kthread)
		ksched->params.edf.active = NULL;

	kclock_gettime(CLOCK_REALTIME, &now);

	if (params->edf.flags & EDF_SET)
	{
		tsched->params.edf.period = params->edf.period;
		tsched->params.edf.relative_deadline = params->edf.deadline;
		tsched->params.edf.flags = params->edf.flags ^ EDF_SET;

		/* create and set periodic alarm */
		edf_create_alarm(kthread, edf_period_alarm,
				   &tsched->params.edf.period_alarm);
		tsched->params.edf.next_run = now;
		time_add(&tsched->params.edf.next_run, &params->edf.period);
		alarm.it_interval = tsched->params.edf.period;
		alarm.it_value = tsched->params.edf.next_run;
		ktimer_settime(tsched->params.edf.period_alarm, TIMER_ABSTIME,
				 &alarm, NULL);

		/* adjust "next_run" and "deadline" for "0" period
		 * first "edf_wait" will set correct values for first period */
		tsched->params.edf.next_run = now;
		time_sub(&tsched->params.edf.next_run, &params->edf.period);

		/* create and set deadline alarm */
		edf_create_alarm(kthread, edf_deadline_alarm,
				   &tsched->params.edf.deadline_alarm);
		tsched->params.edf.active_deadline = now;
		time_add(&tsched->params.edf.active_deadline,
			   &params->edf.deadline);
		TIME_RESET(&alarm.it_interval);
		alarm.it_value = tsched->params.edf.active_deadline;
		ktimer_settime(tsched->params.edf.deadline_alarm,
				 TIMER_ABSTIME, &alarm, NULL);

		/* move thread to edf scheduler */
		kthread_enqueue(kthread, &ksched->params.edf.ready,
				  0, NULL, NULL);
		edf_schedule(ksched);
	}
	else if (params->edf.flags & EDF_WAIT)
	{
		if (edf_check_deadline(kthread))
			return EXIT_FAILURE;

		/* set times for next period */
		if (time_cmp(&now, &tsched->params.edf.next_run) > 0)
		{
			time_add(&tsched->params.edf.next_run,
				   &tsched->params.edf.period);

			tsched->params.edf.active_deadline =
				tsched->params.edf.next_run;
			time_add(&tsched->params.edf.active_deadline,
				   &tsched->params.edf.relative_deadline);

			if (kthread == ksched->params.edf.active)
				ksched->params.edf.active = NULL;

			/* set (separate) alarm for deadline
			 * (periodic alarm is set only once as periodic) */

			TIME_RESET(&alarm.it_interval);
			alarm.it_value = tsched->params.edf.active_deadline;
			ktimer_settime(tsched->params.edf.deadline_alarm,
					 TIMER_ABSTIME, &alarm, NULL);
		}

		/* is task ready for execution, or must wait until next period*/
		if (time_cmp(&tsched->params.edf.next_run, &now) > 0)
		{
			/* wait till "next_run" */
			EDF_LOG("%x [EDF WAIT]", kthread);
			kthread_enqueue(kthread, &ksched->params.edf.wait,
					  0, NULL, NULL);
			edf_schedule(ksched);
		}
		else {
			/* "next_run" has already come,
			 * activate task => move it to "EDF ready tasks"
			 */
			EDF_LOG("%x [EDF READY]", kthread);
			kthread_enqueue(kthread, &ksched->params.edf.ready,
					  0, NULL, NULL);
			edf_schedule(ksched);
		}
	}
	else if (params->edf.flags & EDF_EXIT)
	{
		if (kthread == ksched->params.edf.active)
			ksched->params.edf.active = NULL;

		if (edf_check_deadline(kthread))
		{
			EDF_LOG("%x [EXIT-error]", kthread);
			return EXIT_FAILURE;
		}

		EDF_LOG("%x [EXIT-normal]", kthread);

		if (tsched->params.edf.period_alarm)
		{
			/* disarm timer */
			TIME_RESET(&alarm.it_interval);
			TIME_RESET(&alarm.it_value);
			ktimer_settime(tsched->params.edf.period_alarm, 0,
					 &alarm, NULL);
		}

		if (tsched->params.edf.deadline_alarm)
		{
			/* disarm timer */
			TIME_RESET(&alarm.it_interval);
			TIME_RESET(&alarm.it_value);
			ktimer_settime(tsched->params.edf.deadline_alarm, 0,
					 &alarm, NULL);
		}

		kthread_setschedparam(kthread, SCHED_FIFO, NULL);
	}

	return 0;
}

static int edf_schedule(ksched_t *ksched)
{
	kthread_t *first, *next, *edf_active;
	kthread_sched2_t *sch_first, *sch_next, *ea;

	edf_active = ksched->params.edf.active;
	if (edf_active && !kthread_is_ready(edf_active))
	{
		ksched->params.edf.active = edf_active = NULL;
	}

	first = kthreadq_get(&ksched->params.edf.ready);

	EDF_LOG("%x %x [active, first in queue]", edf_active, first);

	if (!first)
	{
		kthreads_schedule();
		return 0; /* no threads in edf.ready queue, edf.active unch. */
	}

	if (edf_active)
	{
		next = first;
		first = edf_active;
	}
	else {
		next = kthreadq_get_next(first);
	}

	while (first && next)
	{

		sch_first = kthread_get_sched2_param(first);
		sch_next = kthread_get_sched2_param(next);

		if (time_cmp(&sch_first->params.edf.active_deadline,
			&sch_next->params.edf.active_deadline) > 0)
		{
			first = next;
		}

		next = kthreadq_get_next(next);
	}

	if (first && first != edf_active)
	{
		next = kthreadq_remove(&ksched->params.edf.ready, first);
		EDF_LOG("%x removed, %x is now first", next,
			  kthreadq_get(&ksched->params.edf.ready));

		if (edf_active)
		{
			EDF_LOG("%x=>%x [EDF_SCHED_PREEMPT]",
				  edf_active, first);

			/*
			 * change active EDF thread:
			 * -remove it from active/ready list
			 * -put it into edf.ready list
			 */
			if (kthread_is_ready(edf_active))
			{
				if (!kthread_is_active(edf_active))
				{
					kthread_remove_from_ready(edf_active);

					/*
					 * set "deactivated" flag, don't need
					 * another call to "edf_schedule"
					 */
				}
				else {
				ea = kthread_get_sched2_param(edf_active);
				ea->activated = 0;
				}

				kthread_enqueue(edf_active,
						  &ksched->params.edf.ready,
						  0, NULL, NULL);
			}
			/* else = thread is blocked - leave it there */
		}

		ksched->params.edf.active = first;
		EDF_LOG("%x [new active]", first);

		kthread_move_to_ready(first, LAST);
	}

	kthreads_schedule();

	return 0;
}

/*! Timer interrupt for edf */
static void edf_period_alarm(sigval_t sigev_value)
{
	kthread_t *kthread = sigev_value.sival_ptr, *test;
	ksched_t *ksched;

	ASSERT(kthread);

	ksched = ksched2_get(kthread_get_sched_policy(kthread));

	test = kthreadq_remove(&ksched->params.edf.wait, kthread);

	EDF_LOG("%x %x [Period alarm]", kthread, test);

	if (test == kthread)
	{
		if (!edf_check_deadline(kthread))
		{
			EDF_LOG("%x [Waked, moved to edf.ready]", kthread);
			kthread_enqueue(kthread, &ksched->params.edf.ready,
					  0, NULL, NULL);

			edf_schedule(ksched);
		}
		/* else => missed deadline -- handle with deadline timer */
	}
	else {
		/*
		 * thread is not in edf.wait queue, but might be running or its
		 * blocked - it is probable it missed its deadline, but that
		 * will be handled with different timer
		 */
		EDF_LOG("%x [Not in edf.wait. Missed deadline?]", kthread);
	}
}


static void edf_deadline_alarm(sigval_t sigev_value)
{
	kthread_t *kthread = sigev_value.sival_ptr, *test;
	kthread_sched2_t *tsched;
	ksched_t *ksched;
	itimerspec_t alarm;

	ASSERT(kthread);

	ksched = ksched2_get(kthread_get_sched_policy(kthread));
	tsched = kthread_get_sched2_param(kthread);

	test = kthreadq_remove(&ksched->params.edf.wait, kthread);

	EDF_LOG("%x %x [Deadline alarm]", kthread, test);

	if (test == kthread)
	{
		EDF_LOG("%x [Waked, but too late]", kthread);

		kthread_set_syscall_retval(kthread, EXIT_FAILURE);
		kthread_move_to_ready(kthread, LAST);

		if (tsched->params.edf.flags & EDF_TERMINATE)
		{
			EDF_LOG("%x [EDF_TERMINATE]", kthread);
			ktimer_delete(tsched->params.edf.period_alarm);
			tsched->params.edf.period_alarm = NULL;
			ktimer_delete(tsched->params.edf.deadline_alarm);
			tsched->params.edf.deadline_alarm = NULL;
			kthread_set_errno(kthread, ETIMEDOUT);
			kthread_exit(kthread, NULL, TRUE);
		}
		else {
			edf_schedule(ksched);
		}
	}
	else {
	/*
	 * thread is not in edf.wait queue, but might be running or its
	 * blocked - it is probable (almost certain) that it missed deadline
	 */
	EDF_LOG("%x [Not in edf.wait. Missed deadline?]", kthread);

	if (edf_check_deadline(kthread))
	{
		/* what to do if its missed? kill thread? */
		if (tsched->params.edf.flags & EDF_TERMINATE)
		{
			EDF_LOG("%x [EDF_TERMINATE]", kthread);
			ktimer_delete(tsched->params.edf.period_alarm);
			tsched->params.edf.period_alarm = NULL;
			ktimer_delete(tsched->params.edf.deadline_alarm);
			tsched->params.edf.deadline_alarm = NULL;
			kthread_set_errno(kthread, ETIMEDOUT);
			kthread_exit(kthread, NULL, TRUE);
		}
		else if (tsched->params.edf.flags & EDF_CONTINUE)
		{
			/* continue as deadline is not missed */
			EDF_LOG("%x [EDF_CONTINUE]", kthread);
		}
		else if (tsched->params.edf.flags & EDF_SKIP)
		{
			/* skip deadline */
			/* set times for next period */
			EDF_LOG("%x [EDF_SKIP]", kthread);

			time_add(&tsched->params.edf.next_run,
				   &tsched->params.edf.period);

			tsched->params.edf.active_deadline =
					tsched->params.edf.next_run;
			time_add(&tsched->params.edf.active_deadline,
					&tsched->params.edf.relative_deadline);

			if (kthread == ksched->params.edf.active)
				ksched->params.edf.active = NULL;

			TIME_RESET(&alarm.it_interval);
			alarm.it_value = tsched->params.edf.active_deadline;
			ktimer_settime(tsched->params.edf.deadline_alarm,
					 TIMER_ABSTIME, &alarm, NULL);

			alarm.it_interval = tsched->params.edf.period;
			alarm.it_value = tsched->params.edf.next_run;
			ktimer_settime(tsched->params.edf.period_alarm,
					 TIMER_ABSTIME, &alarm, NULL);

			kthread_enqueue(kthread, &ksched->params.edf.ready,
					  0, NULL, NULL);
			edf_schedule(ksched);
		}
	} /* moved 1 tab left for readability */
	}
}

/*!
 * Deactivate thread because:
 * 1. higher priority thread becomes active
 * 2. this thread blocks on some queue (not in edf_ready)
 */
static int edf_thread_deactivate(ksched_t *ksched, kthread_t *kthread)
{
	if (	kthread_is_alive(kthread) && !kthread_is_ready(kthread) &&
		kthread_get_queue(kthread) != &ksched->params.edf.ready &&
		kthread_get_queue(kthread) != &ksched->params.edf.wait)
	{
		/* if kthread is blocked, but not in edf.ready */
		ksched->params.edf.active = NULL;
		edf_schedule(ksched);
	}

	return 0;
}

/*! Check if task hasn't overrun its deadline */
static int edf_check_deadline(kthread_t *kthread)
{
	/* Check if "now" is greater than "active_deadline" */
	timespec_t now;
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);

	kclock_gettime(CLOCK_REALTIME, &now);

	if (time_cmp(&now, &tsched->params.edf.active_deadline) > 0)
	{
		EDF_LOG("%x [DEADLINE OVERRUN]", kthread);
		return EXIT_FAILURE;
	}

	return 0;
}
