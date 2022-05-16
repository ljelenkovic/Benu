/*! POSIX interface to threads
 *  + mutexes + condition variables + semaphores
 */
#define _K_PTHREAD_C_
#define _K_SCHED_

#include "thread.h"
#include "pthread.h"
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

/*! Mutex ------------------------------------------------------------------- */

/*!
 * Initialize mutex object
 * \param mutex Mutex descriptor (user level descriptor)
 * \param mutexattr Mutex parameters
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_mutex_init(pthread_mutex_t *mutex,
			      pthread_mutexattr_t *mutexattr)
{
	kpthread_mutex_t *kmutex;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	kobj = kmalloc_kobject(sizeof(kpthread_mutex_t));
	ASSERT_ERRNO_AND_EXIT(kobj, ENOMEM);
	kmutex = kobj->kobject;

	kmutex->id = k_new_id();
	kmutex->owner = NULL;
	kmutex->flags = 0;
	kmutex->ref_cnt = 1;
	kthreadq_init(&kmutex->queue);

	mutex->ptr = kobj;
	mutex->id = kmutex->id;

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Destroy mutex object
 * \param mutex Mutex descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	kpthread_mutex_t *kmutex;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	kobj = mutex->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);

	kmutex = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(kmutex && kmutex->id == mutex->id, EINVAL);

	ASSERT_ERRNO_AND_EXIT(
		kmutex->owner == NULL /* mutex locked! */ &&
		kthreadq_get(&kmutex->queue) == NULL,
		ENOTEMPTY
	);

	kmutex->ref_cnt--;

	/* additional cleanup here (e.g. if mutex is shared leave it) */
	if (kmutex->ref_cnt)
		SYS_EXIT(EBUSY, EXIT_FAILURE);

	kfree_kobject(kobj);

	mutex->ptr = NULL;
	mutex->id = 0;

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

static int mutex_lock(kpthread_mutex_t *kmutex, kthread_t *kthread);

/*!
 * Lock mutex object
 * \param mutex Mutex descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_mutex_lock(pthread_mutex_t *mutex)
{
	kpthread_mutex_t *kmutex;
	kobject_t *kobj;
	int retval = EXIT_SUCCESS;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	kobj = mutex->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);
	kmutex = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(kmutex && kmutex->id == mutex->id, EINVAL);

	retval = mutex_lock(kmutex, kthread_get_active());

	if (retval != -1)
		kthread_set_syscall_retval(NULL, EXIT_SUCCESS);
	else
		kthread_set_syscall_retval(NULL, EXIT_FAILURE);

	if (retval == 1)
		kthreads_schedule();

	SYS_EXIT(kthread_get_errno(NULL), kthread_get_syscall_retval(NULL));
}

/*! lock mutex; return 0 if locked, 1 if thread blocked, -1 if error */
static int mutex_lock(kpthread_mutex_t *kmutex, kthread_t *kthread)
{
	if (!kmutex->owner)
	{
		/* mutex was not locked, acquire lock on it */
		kmutex->owner = kthread;
		kthread_set_errno(kthread, EXIT_SUCCESS);

		return 0;
	}
	else {
		/* mutex was locked */

		/* recursive locking? */
		if (kmutex->owner == kthread)
		{
			kthread_set_errno(kthread, EDEADLK);
			return -1;
		}

		kthread_set_errno(kthread, EXIT_SUCCESS);
		kthread_enqueue(kthread, &kmutex->queue);

		return 1;
	}
}

/*!
 * Unlock mutex object
 * \param mutex Mutex descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	kpthread_mutex_t *kmutex;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	kobj = mutex->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);
	kmutex = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(kmutex && kmutex->id == mutex->id, EINVAL);

	if (kmutex->owner != kthread_get_active())
	{
		SET_ERRNO(EPERM);
		return EXIT_FAILURE;
	}

	SET_ERRNO(EXIT_SUCCESS);

	kmutex->owner = kthreadq_get(&kmutex->queue);
	if (kmutex->owner)
	{
		kthreadq_release(&kmutex->queue);
		kthreads_schedule();
	}

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*! conditional variables --------------------------------------------------- */

/*!
 * Initialize conditional variable object
 * \param cond conditional variable descriptor (user level descriptor)
 * \param condattr conditional variable descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *condattr)
{
	kpthread_cond_t *kcond;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(cond, EINVAL);

	kobj = kmalloc_kobject(sizeof(kpthread_cond_t));
	ASSERT_ERRNO_AND_EXIT(kobj, ENOMEM);
	kcond = kobj->kobject;

	kcond->id = k_new_id();
	kcond->flags = 0;
	kcond->ref_cnt = 1;
	kthreadq_init(&kcond->queue);

	cond->ptr = kobj;
	cond->id = kcond->id;

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Destroy conditional variable object
 * \param cond conditional variable descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_destroy(pthread_cond_t *cond)
{
	kpthread_cond_t *kcond;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(cond, EINVAL);

	kobj = cond->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);
	kcond = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(kcond && kcond->id == cond->id, EINVAL);

	kcond->ref_cnt--;

	/* additional cleanup here (e.g. if cond.var. is shared leave it) */
	if (kcond->ref_cnt)
		SYS_EXIT(EBUSY, EXIT_FAILURE);

	kfree_kobject(kobj);

	cond->ptr = NULL;
	cond->id = 0;

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Wait on conditional variable
 * \param cond conditional variable descriptor (user level descriptor)
 * \param mutex Mutex descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	kpthread_cond_t *kcond;
	kpthread_mutex_t *kmutex;
	kobject_t *kobj_cond, *kobj_mutex;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(cond && mutex, EINVAL);

	kobj_cond = cond->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj_cond, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj_cond->list),
				EINVAL);
	kcond = kobj_cond->kobject;
	ASSERT_ERRNO_AND_EXIT(kcond && kcond->id == cond->id, EINVAL);

	kobj_mutex = mutex->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj_mutex, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj_mutex->list),
				EINVAL);
	kmutex = kobj_mutex->kobject;
	ASSERT_ERRNO_AND_EXIT(kmutex && kmutex->id == mutex->id, EINVAL);

	ASSERT_ERRNO_AND_EXIT(kmutex->owner == kthread_get_active(), EPERM);

	kthread_set_errno(NULL, EXIT_SUCCESS);
	kthread_set_syscall_retval(NULL, EXIT_SUCCESS);

	/* move thread in conditional variable queue */
	kthread_enqueue(NULL, &kcond->queue);

	/* save reference to mutex object */
	kthread_set_private_param(NULL, kobj_mutex);

	/* release mutex */
	kmutex->owner = kthreadq_get(&kmutex->queue);
	if (kmutex->owner)
		kthreadq_release(&kmutex->queue);

	kthreads_schedule();

	SYS_EXIT(kthread_get_errno(NULL), kthread_get_syscall_retval(NULL));
}

static int cond_release(pthread_cond_t *cond, int release_all);

/*!
 * Restart thread waiting on conditional variable
 * \param cond conditional variable descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_signal(pthread_cond_t *cond)
{
	return cond_release(cond, FALSE);
}

/*!
 * Restart all threads waiting on conditional variable
 * \param cond conditional variable descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_broadcast(pthread_cond_t *cond)
{
	return cond_release(cond, TRUE);
}

static int cond_release(pthread_cond_t *cond, int release_all)
{
	kpthread_cond_t *kcond;
	kpthread_mutex_t *kmutex;
	kobject_t *kobj_cond, *kobj_mutex;
	kthread_t *kthread;
	int released = 0;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(cond, EINVAL);

	kobj_cond = cond->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj_cond, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj_cond->list),
				EINVAL);
	kcond = kobj_cond->kobject;
	ASSERT_ERRNO_AND_EXIT(kcond && kcond->id == cond->id, EINVAL);

	kthread_set_errno(kthread_get_active(), EXIT_SUCCESS);

	while ((kthread = kthreadq_remove(&kcond->queue, NULL)))
	{
		kobj_mutex = kthread_get_private_param(kthread);
		kmutex = kobj_mutex->kobject;

		if (mutex_lock(kmutex, kthread) == 0) {
			kthread_move_to_ready(kthread, LAST);
			released++;
		}

		/* release only one? */
		if (!release_all)
			break;
	}

	if (released)
		kthreads_schedule();

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}


/*! Semaphore --------------------------------------------------------------- */

/*!
 * Initialize semaphore object
 * \param sem Semaphore descriptor (user level descriptor)
 * \param pshared Shall semaphore object be shared between processes
 * \param value Initial semaphore value
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sem_init(sem_t *sem, int pshared, int value)
{
	ksem_t *ksem;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	kobj = kmalloc_kobject(sizeof(ksem_t));
	ASSERT_ERRNO_AND_EXIT(kobj, ENOMEM);
	ksem = kobj->kobject;

	ksem->id = k_new_id();
	ksem->sem_value = value;
	ksem->last_lock = NULL;
	ksem->flags = 0;
	ksem->ref_cnt = 1;
	kthreadq_init(&ksem->queue);

	if (pshared)
		ksem->flags |= PTHREAD_PROCESS_SHARED;

	sem->ptr = kobj;
	sem->id = ksem->id;

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Destroy semaphore object
 * \param sem Semaphore descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sem_destroy(sem_t *sem)
{
	ksem_t *ksem;
	kobject_t *kobj;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	kobj = sem->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);
	ksem = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ksem && ksem->id == sem->id, EINVAL);

	ASSERT_ERRNO_AND_EXIT(kthreadq_get(&ksem->queue) == NULL, ENOTEMPTY);

	ksem->ref_cnt--;

	/* additional cleanup here (e.g. if semaphore is shared leave it) */
	if (ksem->ref_cnt)
		SYS_EXIT(EBUSY, EXIT_FAILURE);

	kfree_kobject(kobj);

	sem->ptr = NULL;
	sem->id = 0;

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Decrement (lock) semaphore value by 1 (if not 0 when thread is blocked)
 * \param sem Semaphore descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sem_wait(sem_t *sem)
{
	ksem_t *ksem;
	kobject_t *kobj;
	kthread_t *kthread;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	kthread = kthread_get_active();

	kobj = sem->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);
	ksem = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ksem && ksem->id == sem->id, EINVAL);

	kthread_set_errno(kthread, EXIT_SUCCESS);
	kthread_set_syscall_retval(kthread, EXIT_SUCCESS);

	if (ksem->sem_value > 0)
	{
		ksem->sem_value--;
		ksem->last_lock = kthread;
	}
	else {
		kthread_enqueue(kthread, &ksem->queue);
		kthreads_schedule();
	}

	SYS_EXIT(kthread_get_errno(NULL), kthread_get_syscall_retval(NULL));
}

/*!
 * Increment (lock) semaphore value by 1 (or unblock one thread that is blocked)
 * \param sem Semaphore descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sem_post(sem_t *sem)
{
	ksem_t *ksem;
	kobject_t *kobj;
	kthread_t *kthread, *released;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	kthread = kthread_get_active();

	kobj = sem->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&kobjects, &kobj->list),
				EINVAL);
	ksem = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ksem && ksem->id == sem->id, EINVAL);

	kthread_set_errno(kthread, EXIT_SUCCESS);
	kthread_set_syscall_retval(kthread, EXIT_SUCCESS);

	released = kthreadq_get(&ksem->queue); /* first to release */

	if (!released || ksem->sem_value < 0)
	{
		/* if initial semaphore value (set by sem_init) was negative,
		 * semaphore will not release threads until its value
		 * reaches zero (small extension of POSIX semaphore) */
		ksem->sem_value++;
	}
	else {
		kthreadq_release(&ksem->queue);
		kthreads_schedule();
	}

	SYS_EXIT(kthread_get_errno(NULL), kthread_get_syscall_retval(NULL));
}
