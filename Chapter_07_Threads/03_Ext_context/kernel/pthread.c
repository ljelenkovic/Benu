/*! POSIX interface to threads
 *  + mutexes + condition variables + semaphores
 */
#define _K_PTHREAD_C_
#define _K_SCHED_

#include "thread.h"
#include <kernel/pthread.h>

#include "memory.h"
#include "sched.h"
#include <lib/string.h>
#include <kernel/errno.h>

/*! Threads ----------------------------------------------------------------- */

/*!
 * Create new thread (params on user stack!)
 * \param thread User level thread descriptor
 * \param attr Thread attributes
 * \param start_routine Starting function for new thread
 * \param arg Parameter sent to starting function
 * (parameters are on calling thread stack)
 */
int sys__pthread_create(pthread_t *thread, pthread_attr_t *attr,
			  void *(*start_routine)(void *), void *arg)
{
	kthread_t *kthread;
	uint flags = 0;
	int sched_policy = SCHED_FIFO;
	int sched_priority = THREAD_DEF_PRIO;
	void *stackaddr = NULL;
	size_t stacksize = 0;

	SYS_ENTRY();

	if (attr)
	{
		flags = attr->flags;
		sched_policy = attr->sched_policy;
		sched_priority = attr->sched_params.sched_priority;
		stackaddr = attr->stackaddr;
		stacksize = attr->stacksize;

		ASSERT_ERRNO_AND_EXIT(
			sched_policy >= 0 && sched_policy < SCHED_NUM,
			ENOTSUP
		);
		ASSERT_ERRNO_AND_EXIT(
			sched_priority >= THREAD_MIN_PRIO &&
			sched_priority <= THREAD_MAX_PRIO,
			ENOMEM
		);

		/* if (flags & SOMETHING) change attributes ... */
	}

	kthread = kthread_create(start_routine, arg, flags,
				   sched_policy, sched_priority,
				   stackaddr, stacksize);

	ASSERT_ERRNO_AND_EXIT(kthread, ENOMEM);

	if (thread)
	{
		thread->ptr = kthread;
		thread->id = kthread_get_id(kthread);
	}

	kthreads_schedule();

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * End current thread (exit from it)
 * \param retval Pointer to exit status
 */
int sys__pthread_exit(void *retval)
{
	SYS_ENTRY();

	kthread_exit(kthread_get_active(), retval);

	ASSERT(FALSE); /* should not return here! */

	SYS_EXIT(EXIT_FAILURE, EXIT_FAILURE);
}

/*!
 * Wait for thread termination
 * \param thread Thread descriptor (user level descriptor)
 * \param retval Where to store exit status of joined thread
 * \return 0 if thread already gone; -1 if not finished and 'wait' not set;
 *         'thread exit status' otherwise
 */
int sys__pthread_join(pthread_t *thread, void **retval)
{
	kthread_t *kthread;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(thread, ESRCH);

	kthread = thread->ptr;

	if (kthread_get_id(kthread) != thread->id)
	{
		/* at 'kthread' is now something else */
		SYS_EXIT(ESRCH, EXIT_FAILURE);
	}
	else if (kthread_is_alive(kthread))
	{
		kthread_set_errno(NULL, EXIT_SUCCESS);
		kthread_set_syscall_retval(NULL, EXIT_SUCCESS);

		kthread_set_private_param(kthread_get_active(), retval);

		kthread_wait_thread(NULL, kthread);

		kthreads_schedule();

		SYS_EXIT(kthread_get_errno(NULL),
			   kthread_get_syscall_retval(NULL));
	}
	else {
		/* target thread is passive, collect status and free descr. */
		kthread_collect_status(kthread, retval);

		SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
	}
}

/*! Return calling thread descriptor
 * \param thread Thread descriptor (user level descriptor)
 * \return 0
 */
int sys__pthread_self(pthread_t *thread)
{
	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(thread, ESRCH);

	thread->ptr = kthread_get_active();
	thread->id = kthread_get_id(NULL);

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Change scheduling parameters
 * \param thread User level thread descriptor
 * \param policy Thread scheduling policy
 * \param param Additional scheduling parameters (when policy != SCHED_FIFO)
 * \return 0
 */
int sys__pthread_setschedparam(pthread_t *thread, int policy,
				 sched_param_t *param)
{
	kthread_t *kthread;
	int retval;

	SYS_ENTRY();

	kthread = thread->ptr;
	ASSERT_ERRNO_AND_EXIT(kthread, EINVAL);
	ASSERT_ERRNO_AND_EXIT(kthread_get_id(kthread) == thread->id, ESRCH);
	ASSERT_ERRNO_AND_EXIT(kthread_is_alive(kthread), ESRCH);

	ASSERT_ERRNO_AND_EXIT(policy >= 0 && policy < SCHED_NUM, EINVAL);

	if (param)
	{
		ASSERT_ERRNO_AND_EXIT(
			param->sched_priority >= THREAD_MIN_PRIO &&
			param->sched_priority <= THREAD_MAX_PRIO, EINVAL);
	}

	retval = kthread_setschedparam(kthread, policy, param);
	if (retval == EXIT_SUCCESS)
		SYS_EXIT(EXIT_SUCCESS, retval);
	else
		SYS_EXIT(retval, EXIT_FAILURE);
}

/*! Set and get current thread error status */
int sys__set_errno(int errno)
{
	SYS_ENTRY();

	kthread_set_errno(NULL, errno);

	SYS_RETURN(EXIT_SUCCESS);
}
int sys__get_errno()
{
	SYS_ENTRY();
	SYS_RETURN(kthread_get_errno(NULL));
}

int sys__get_errno_ptr(int **errno)
{
	SYS_ENTRY();

	if (errno)
	{
		*errno = kthread_get_errno_ptr(NULL);
		SYS_RETURN(EXIT_SUCCESS);
	}
	else {
		SYS_RETURN(EXIT_FAILURE);
	}
}
