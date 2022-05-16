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
	}

	/* process pending signals (if any) */
	ksignal_process_pending(kthread_get_active());

	if (curr != kthread_get_active())
		kthread_switch_to_thread(curr, kthread_get_active());
	/* else => continue with current thread */
}
