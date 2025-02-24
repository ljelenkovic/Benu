/*! Thread management */
#pragma once

#include <kernel/thread.h>
#include <lib/list.h>

/*! kernel thread descriptor (used only as a pointer outside thread.c) */
#ifndef _K_THREAD_C_
typedef void kthread_t;
#else
struct _kthread_t_;
typedef struct _kthread_t_ kthread_t;
#endif /* _K_THREAD_C_ */

/*! dependencies from other subsystems */
#include "memory.h"
#include "sched.h"
#include "time.h"

/*! Interface for kernel (this and other subsystems) ------------------------ */
void kthreads_init();
kthread_t *kthread_create(void *start_routine, void *arg, uint flags,
      int sched_policy, int sched_priority, void *stackaddr, size_t stacksize);

/*! suspend thread on delay */
int kthread_suspend(kthread_t *, void *wakeup_action, void *);

/*! remove thread */
int kthread_exit(kthread_t *kthread, void *exit_status);
int kthread_exit2(kthread_t *kthread, void *exit_status);
void kthread_wait_thread(kthread_t *waiting, kthread_t *waited);
void kthread_collect_status(kthread_t *waited, void **retval);
void kthread_switch_to_thread(kthread_t *from, kthread_t *to);
void kthread_cleanup(kthread_t *kthread);

/*! Thread queue manipulation - advanced operations */
void kthread_enqueue(kthread_t *kthread, kthread_q *q_id);
int kthreadq_release(kthread_q *q_id);
int kthreadq_release_all(kthread_q *q_id);

/*! Thread queue manipulation - basic operations */
void kthreadq_init(kthread_q *q);
void kthreadq_append(kthread_q *q, kthread_t *kthread);
void kthreadq_prepend(kthread_q *q, kthread_t *kthread);
kthread_t *kthreadq_remove(kthread_q *q, kthread_t *kthread);
kthread_t *kthreadq_get(kthread_q *q);
kthread_t *kthreadq_get_next(kthread_t *kthread);

/*! set thread scheduling parameters */
int kthread_setschedparam(kthread_t *kthread, int policy,
			    sched_param_t *param);

/*! get thread scheduling policy */
int kthread_get_sched_policy(kthread_t *kthread);

/*! get/set (set=change) thread priority */
int kthread_get_prio(kthread_t *kthread);
int kthread_set_prio(kthread_t *kthread, int prio);

/*! Get-ers and Set-ers ----------------------------------------------------- */
int kthread_is_active(kthread_t *kthread);
int kthread_is_ready(kthread_t *kthread);
int kthread_is_alive(kthread_t *kthread);
int kthread_is_passive(kthread_t *kthread);
int kthread_is_suspended(kthread_t *, void **func, void **param);
int kthread_check_kthread(kthread_t *kthread);

int kthread_get_id(kthread_t *kthread);
kthread_t *kthread_get_active();
void *kthread_get_context(kthread_t *thread);
kthread_t *kthread_get_descriptor(pthread_t *thr);

/* save extra parameter when blocking thread */
void kthread_set_private_param(kthread_t *kthread, void *qdata);
void *kthread_get_private_param(kthread_t *kthread);

/*! errno and return value */
void kthread_set_errno(kthread_t *kthread, int error_number);
int kthread_get_errno(kthread_t *kthread);
int *kthread_get_errno_ptr(kthread_t *kthread);
void kthread_set_syscall_retval(kthread_t *kthread, int ret_val);
int kthread_get_syscall_retval(kthread_t *kthread);

/*! display active & ready threads info on console */
int kthread_info();

#ifdef _K_SCHED_
void kthread_set_active(kthread_t *kthread);
void kthread_mark_ready(kthread_t *kthread);
void kthread_set_queue(kthread_t *kthread, kthread_q *queue);
kthread_q *kthread_get_queue(kthread_t *kthread);
#endif /* _K_SCHED_ */

#ifdef _K_THREAD_C_ /* rest of the file is only for kernel/thread.c */

#include <arch/context.h>

/*! Thread descriptor */
struct _kthread_t_
{
	id_t	   id;
		   /* thread id (number) */

	int	   state;
		   /* thread state (active,ready,wait,susp.) */

	int	   flags;
		   /* various flags (as detachable, cancelable) */

	context_t  context;
		   /* storage for thread context */

	int	   retval;
		   /* return value from system call (when changed by others) */

	int	   errno;
		   /* exit status of last system function call */

	void	  *exit_status;
		   /* status with which thread exited (pointer!) */

	void	 (*cancel_suspend_handler)(kthread_t *, void *);
	void	  *cancel_suspend_param;
	/* cancellation handler - when premature cancellation occurs while
	 * thread is suspended; used to perform required actions at cancellation
	 * event, e.g. when sleep is interrupted */

	void	  *pparam;
		   /* temporary storage for one private parameter;
		    * to be used only when thread is blocked to store single
		    * parameter used by kernel only - not for private storage */

	void	  *stack;
	uint	   stack_size;
		   /* stack address and size (for deallocation) */

	int	   sched_policy;
		   /* scheduling policy */
	int	   sched_priority;
		   /* priority - primary scheduling parameter */

	kthread_q  *queue;
		   /* in which queue thread is (if not active) */

	kthread_q  join_queue;
		   /* queue for threads waiting for this to end */

	list_h     list;
		   /* list element for "thread state" list */

	list_h	   all;
		   /* list element for list of all threads */

	int	   ref_cnt;
		   /* reference counter */
};

/*! Thread states */
enum {
	THR_STATE_ACTIVE = 1,
	THR_STATE_READY,
	THR_STATE_WAIT,
	THR_STATE_SUSPENDED,
	THR_STATE_PASSIVE	/* when just descriptor is left of thread */
};

#endif	/* _K_THREAD_C_ */
