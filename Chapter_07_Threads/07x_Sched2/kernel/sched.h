/*! Scheduler interfaces
 *
 * Master scheduler implemented is priority scheduler with FIFO for threads with
 * same priority.
 *
 * Secondary schedulers can influence scheduling of their threads by adjusting
 * priority of tasks (threads) they are "scheduling".
 * For more information on how to implement particular scheduler look at example
 * given with Round Robin scheduling (sched_rr.h/c).
 */
#pragma once

#include <kernel/thread.h>

/*! ------------------------------------------------------------------------- */
/*! Master scheduler = priority + FIFO -------------------------------------- */
/*! ------------------------------------------------------------------------- */
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



/*! ------------------------------------------------------------------------- */
/*! Secondary schedulers (visible only to schedulers and thread.c) ---------- */
/*! ------------------------------------------------------------------------- */
#ifdef _K_SCHED_


#include "sched_rr.h"
#include "sched_edf.h"

/*! Thread specific data/interface (for scheduler, nor for user) ------------ */

/*! Union of per thread specific data types required by all schedulers */
typedef union _kthread_sched_params_t_
{
	ksched_rr_thread_params     rr;
		      /* Round Robin per thread data */

	ksched_edf_thread_params_t  edf;
		      /* Earliest Deadline First per thread data */

	/* add others thread scheduling parameters for other schedulers that
	   require parameters */
}
kthread_sched_params_t;

/*! Scheduling parameters for each thread (included in thread descriptor) */
typedef struct _kthread_sched2_t_
{
	int  activated;
	     /* disable multiple activation/deactivation calls when thread
	      * becomes active (or stop being active) */

	kthread_sched_params_t  params;
				/* scheduler per thread specific data */
}
kthread_sched2_t;

#include "thread.h"


struct _ksched_t_;
typedef struct _ksched_t_ ksched_t;

ksched_t *ksched2_get(int sched_policy);

int ksched2_thread_add(kthread_t *kthread, int sched_policy,
			 int sched_priority, sched_supp_t *sched_param);
int ksched2_thread_remove(kthread_t *kthread);
int ksched2_activate_thread(kthread_t *kthread);
int ksched2_deactivate_thread(kthread_t *kthread);
int ksched2_setsched_param(kthread_t *kthread, sched_supp_t *sched_param);

void ksched2_schedule(int sched_policy);


/*! Global scheduler specific data/interface -------------------------------- */
/*! Union of per scheduler specific data types required */
typedef union _ksched_params_t_
{
	ksched_rr_t   rr;
		      /* Round Robin global data */

	ksched_edf_t  edf;
		      /* Earliest Deadline First data */

	/* add others thread scheduling parameters for other schedulers that
	   require parameters */
}
ksched_params_t;

/*! Secondary scheduler interface */
struct _ksched_t_
{
	int  sched_id;
	     /* scheduler ID, e.g. SCHED_FIFO */

	int (*init)(ksched_t *ksched);
	     /* initialize scheduler */

	int (*schedule)(ksched_t *ksched);
	     /* schedule - pick next active thread */

	int (*thread_add)(ksched_t *ksched, kthread_t *kthread,
			     int sched_priority, sched_supp_t *sched_param);
	/* actions when thread is created or when it switch to this scheduler */

	int (*thread_remove)(ksched_t *ksched, kthread_t *kthread);
	/* actions when thread is removed from this scheduler */

	int (*thread_activate)(ksched_t *ksched, kthread_t *kthread);
	     /* actions when thread is to become active */

	int (*thread_deactivate)(ksched_t *ksched, kthread_t *kthread);
	     /* actions when thread stopped to be active */

	int (*set_thread_sched_parameters)(
	       ksched_t *ksched, kthread_t *kthread, sched_supp_t *param);
	     /* set scheduler specific parameters to thread */

	int (*get_thread_sched_parameters)(
	       ksched_t *ksched, kthread_t *kthread, sched_supp_t *param);
	     /* get scheduler specific parameters from thread */

	ksched_params_t  params;
			 /* scheduler specific data */
};

#endif /* _K_SCHED_ */
