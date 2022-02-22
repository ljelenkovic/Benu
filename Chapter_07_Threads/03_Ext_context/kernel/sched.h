/*! Scheduler interfaces
 *
 * Implemented scheduler is priority based scheduler with FIFO for threads with
 * same priority.
 */
#pragma once

#include <kernel/thread.h>

#include "thread.h"

void ksched_init();
void kthread_move_to_ready(kthread_t *kthread, int where);
kthread_t *kthread_remove_from_ready(kthread_t *kthread);
void kthreads_schedule();

#ifdef _K_SCHED_C_

/*! ready thread data structure */
typedef struct _sched_ready_t_
{
	int	    prio_levels;
		    /* priority levels == PRIO_LEVELS macro defined
		     * in config.ini */

	kthread_q  *rq;
		    /* array of ready queues; each array element one list for
		     * one priority; array index is priority */

	uint	   *mask;
	uint	    mask_len;
		    /* bit mask for O(1) searching for highest priority thread*/
}
sched_ready_t;

#endif /* _K_SCHED_C_ */
