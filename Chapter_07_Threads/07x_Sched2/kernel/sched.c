/*! Secondary scheduler general functions */
#define _K_SCHED_C_
#define _K_SCHED_

#include "sched.h"
#include "thread.h"

#include "memory.h"
#include <kernel/errno.h>
#include <arch/context.h>
#include <types/bits.h>
#include <lib/list.h>

static void ksched2_init();

/*! ------------------------------------------------------------------------- */
/*! Master scheduler = priority + FIFO -------------------------------------- */
/*! ------------------------------------------------------------------------- */

/*! ready threads */
static sched_ready_t ready;

#define UINT_SIZE	(8 * sizeof(uint))

/*! initialize data structure for ready threads */
void ksched_init()
{
	int i;

	ready.prio_levels = PRIO_LEVELS;
	ready.rq = kmalloc(ready.prio_levels * sizeof(kthread_q));

	ready.mask_len = (ready.prio_levels + UINT_SIZE - 1) / UINT_SIZE;
	ready.mask = kmalloc(ready.mask_len * sizeof(uint));

	/* queue for ready threads is empty */
	for (i = 0; i < ready.prio_levels; i++)
		kthreadq_init(&ready.rq[i]);

	for (i = 0; i < ready.mask_len; i++)
		ready.mask[i] = 0;

	ksched2_init();
}

/*!
 * Move given thread (its descriptor) to ready threads
 * (as last or first in its priority queue)
 */
void kthread_move_to_ready(kthread_t *kthread, int where)
{
	int i, j, prio;

	ASSERT(kthread);

	prio = kthread_get_prio(kthread);

	kthread_mark_ready(kthread);
	kthread_set_queue(kthread, &ready.rq[prio]);

	if (where == LAST)
		kthreadq_append(&ready.rq[prio], kthread);
	else
		kthreadq_prepend(&ready.rq[prio], kthread);

	/* mark that list as not empty */
	i = prio / UINT_SIZE;
	j = prio % UINT_SIZE;
	ready.mask[i] |= (uint)(1 << j);
}

/*! Remove given thread (its descriptor) from ready threads */
kthread_t *kthread_remove_from_ready(kthread_t *kthread)
{
	int i, j, prio;

	if (!kthread)
		return NULL;

	prio = kthread_get_prio(kthread);

	if (kthreadq_remove(&ready.rq[prio], kthread) != kthread)
		return NULL;

	/* no more ready threads in list? */
	if (kthreadq_get(&ready.rq[prio]) == NULL)
	{
		i = prio / UINT_SIZE;
		j = prio % UINT_SIZE;

		ready.mask[i] &= ~((uint)(1 << j));
	}

	return kthread;
}

/*! Find and return highest priority thread in ready list */
static kthread_t *get_first_ready()
{
	int i, first;

	for (i = ready.mask_len - 1; i >= 0; i--)
	{
		if (ready.mask[i])
		{
			first = i * UINT_SIZE + msb_index(ready.mask[i]);
			return kthreadq_get(&ready.rq[first]);
		}
	}

	return NULL;
}

/*!
 * Select ready thread with highest priority  as active
 * - if different from current, move current into ready queue (id not NULL) and
 *   move selected thread from ready queue to active queue
 */
void kthreads_schedule()
{
	kthread_t *curr, *next = NULL;

	curr = kthread_get_active();
	next = get_first_ready();

	/* must exist an thread to return to, 'curr' or first from 'ready' */
	ASSERT((curr && kthread_is_active(curr)) || next);

	if (!sys__feature(FEATURE_SCHEDULER, FEATURE_GET, 0) &&
		curr && kthread_is_active(curr))
		return;/*scheduler disabled, don't switch from current thread */

	if (!curr || !kthread_is_active(curr) ||
		kthread_get_prio(curr) < kthread_get_prio(next))
	{
		if (curr && !kthread_is_passive(curr)) /* deactivate curr */
		{
			ksched2_deactivate_thread(curr);

			/* move last active to ready queue, if still ready */
			if (kthread_is_active(curr))
				kthread_move_to_ready(curr, LAST);

			/* deactivation might change ready thread list */
			next = get_first_ready();
			ASSERT(next);
		}

		/* activate next */
		next = kthread_remove_from_ready(next);
		ASSERT(next);

		kthread_set_active(next);

		ksched2_activate_thread(next);
	}

	/* process pending signals (if any) */
	ksignal_process_pending(kthread_get_active());

	if (curr != kthread_get_active())
		kthread_switch_to_thread(curr, kthread_get_active());
	/* else => continue with current thread */
}

/*! ------------------------------------------------------------------------- */
/*! Secondary schedulers ---------------------------------------------------- */
/*! ------------------------------------------------------------------------- */

extern ksched_t ksched_rr;
extern ksched_t ksched_edf;

/*! Statically defined schedulers (could be easily extended to dynamically) */
static ksched_t *ksched[] = {
	NULL,		/* SCHED_FIFO */
	&ksched_rr,	/* SCHED_RR */
	&ksched_edf	/* SCHED_EDF */
};

/*! Initialize all (known) schedulers (called from 'kthreads_init') */
static void ksched2_init()
{
	int i;

	for (i = 0; i < SCHED_NUM; i++)
		if (ksched[i] && ksched[i]->init)
			ksched[i]->init(ksched[i]);
}

/*! Get pointer to ksched_t parameters for requested scheduling policy */
ksched_t *ksched2_get(int sched_policy)
{
	ASSERT(sched_policy >= 0 && sched_policy < SCHED_NUM);

	return ksched[sched_policy];
}

/*! Add thread to scheduling policy (if required by policy) */
int ksched2_thread_add(kthread_t *kthread, int sched_policy,
			 int sched_priority, sched_supp_t *sched_param)
{
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);

	ASSERT(sched_policy >= 0 && sched_policy < SCHED_NUM);

	tsched->activated = 0;

	if (ksched[sched_policy] && ksched[sched_policy]->thread_add)
		ksched[sched_policy]->thread_add(
		ksched[sched_policy], kthread, sched_priority, sched_param);

	return 0;
}

/*! Remove thread from scheduling policy (if required by policy) */
int ksched2_thread_remove(kthread_t *kthread)
{
	int sched_policy = kthread_get_sched_policy(kthread);

	if (ksched[sched_policy] && ksched[sched_policy]->thread_remove)
		ksched[sched_policy]->thread_remove(ksched[sched_policy],
						      kthread);

	return 0;
}

/*! Actions to be performed when thread is to become active */
int ksched2_activate_thread(kthread_t *kthread)
{
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);
	int activated = tsched->activated;
	int sched = kthread_get_sched_policy(kthread);

	if (activated == 0)
	{
		if (ksched[sched] && ksched[sched]->thread_activate)
			ksched[sched]->thread_activate(ksched[sched],
							 kthread);

		tsched->activated = 1;
	}

	return activated;
}

/*! Actions to be performed when thread is removed as active */
int ksched2_deactivate_thread(kthread_t *kthread)
{
	kthread_sched2_t *tsched = kthread_get_sched2_param(kthread);
	int activated = tsched->activated;
	int sched = kthread_get_sched_policy(kthread);

	if (activated == 1)
	{
		if (ksched[sched] && ksched[sched]->thread_deactivate)
			ksched[sched]->thread_deactivate(ksched[sched],
							   kthread);

		tsched->activated = 0;
	}

	return activated;
}

/*! Change (set) scheduling parameters (extra parameters) */
int ksched2_setsched_param(kthread_t *kthread, sched_supp_t *sched_param)
{
	int sched = kthread_get_sched_policy(kthread);

	if (ksched[sched] && ksched[sched]->set_thread_sched_parameters)
		ksched[sched]->set_thread_sched_parameters(
		ksched[sched], kthread, sched_param);

	return 0;
}

/*! Reschedule within given scheduler */
void ksched2_schedule(int sched_policy)
{
	ASSERT(sched_policy >= 0 && sched_policy < SCHED_NUM);

	if (ksched[sched_policy] && ksched[sched_policy]->schedule)
			ksched[sched_policy]->schedule(ksched[sched_policy]);
}
