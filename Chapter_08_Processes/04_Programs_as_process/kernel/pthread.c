/*! POSIX interface to threads
 *  + mutexes + condition variables + semaphores + messages
 */
#define _K_PTHREAD_C_
#define _K_SCHED_

#include "thread.h"
#include "pthread.h"
#include <kernel/pthread.h>

#include "memory.h"
#include "sched.h"
#include <arch/syscall.h>
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
int sys__pthread_create(void *p)
{
	pthread_t *thread;
	pthread_attr_t *attr;
	void *(*start_routine)(void *);
	void *arg;

	kthread_t *kthread;
	uint flags = 0;
	int sched_policy = SCHED_FIFO;
	int sched_priority = THREAD_DEF_PRIO;
	void *stackaddr = NULL;
	size_t stacksize = 0;

	thread = *((pthread_t **) p);		p += sizeof(pthread_t *);
	attr = *((pthread_attr_t **) p);	p += sizeof(pthread_attr_t *);
	start_routine = *((void **) p);	p += sizeof(void *);
	arg = *((void **) p);

	if (attr)
	{
		attr = U2K_GET_ADR(attr, kthread_get_process(NULL));
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
	}

	kthread = kthread_create(start_routine, arg, flags,
				   sched_policy, sched_priority,
				   stackaddr, stacksize,
				   kthread_get_process(NULL)
 				);

	ASSERT_ERRNO_AND_EXIT(kthread, ENOMEM);

	if (thread)
	{
		thread = U2K_GET_ADR(thread, kthread_get_process(NULL));
		thread->ptr = kthread;
		thread->id = kthread_get_id(kthread);
	}

	SET_ERRNO(EXIT_SUCCESS);

	kthreads_schedule();

	return EXIT_SUCCESS;
}

/*!
 * End current thread (exit from it)
 * \param retval Pointer to exit status
 */
int sys__pthread_exit(void *p)
{
	void *retval;

	retval = *((void **) p);
	kthread_exit(kthread_get_active(), retval, FALSE);

	return 0;
}

/*!
 * Wait for thread termination
 * \param thread Thread descriptor (user level descriptor)
 * \param retval Where to store exit status of joined thread
 * \return 0 if thread already gone; -1 if not finished and 'wait' not set;
 *         'thread exit status' otherwise
 */
int sys__pthread_join(void *p)
{
	pthread_t *thread;
	void **retval;

	kthread_t *kthread;
	int ret_value = 0;

	thread = *((pthread_t **) p);		p += sizeof(pthread_t *);
	retval = *((void ***) p);

	ASSERT_ERRNO_AND_EXIT(thread, ESRCH);
	thread = U2K_GET_ADR(thread, kthread_get_process(NULL));
	ASSERT_ERRNO_AND_EXIT(thread->ptr, ESRCH);

	if (retval)
		retval = U2K_GET_ADR(retval, kthread_get_process(NULL));

	kthread = thread->ptr;

	if (kthread_get_id(kthread) != thread->id)
	{
		/* at 'kthread' is now something else */
		ret_value = EXIT_FAILURE;
		SET_ERRNO(ESRCH);
	}
	else if (kthread_is_alive(kthread))
	{
		ret_value = EXIT_SUCCESS;
		SET_ERRNO(EXIT_SUCCESS);
		kthread_set_private_param(kthread_get_active(), retval);

		kthread_wait_thread(NULL, kthread);

		kthreads_schedule();
	}
	else {
		/* target thread is passive, collect status and free descr. */
		ret_value = EXIT_SUCCESS;
		SET_ERRNO(EXIT_SUCCESS);

		kthread_collect_status(kthread, retval);
	}

	return ret_value;
}

/*! Return calling thread descriptor
 * \param thread Thread descriptor (user level descriptor)
 * \return 0
 */
int sys__pthread_self(void *p)
{
	pthread_t *thread;

	thread = U2K_GET_ADR(*((void **) p), kthread_get_process(NULL));

	ASSERT_ERRNO_AND_EXIT(thread, ESRCH);

	thread->ptr = kthread_get_active();
	thread->id = kthread_get_id(NULL);

	EXIT(EXIT_SUCCESS);
}

/*!
 * Change scheduling parameters
 * \param thread User level thread descriptor
 * \param policy Thread scheduling policy
 * \param param Additional scheduling parameters (when policy != SCHED_FIFO)
 * \return 0
 */
int sys__pthread_setschedparam(void *p)
{
	pthread_t *thread;
	int policy;
	sched_param_t *param;

	kthread_t *kthread;

	thread = *((pthread_t **) p);		p += sizeof(pthread_t *);
	policy = *((int *) p);		p += sizeof(int);
	param =  *((sched_param_t **) p);

	thread = U2K_GET_ADR(thread, kthread_get_process(NULL));

	kthread = thread->ptr;
	ASSERT_ERRNO_AND_EXIT(kthread, EINVAL);
	ASSERT_ERRNO_AND_EXIT(kthread_get_id(kthread) == thread->id, ESRCH);
	ASSERT_ERRNO_AND_EXIT(kthread_is_alive(kthread), ESRCH);

	ASSERT_ERRNO_AND_EXIT(policy >= 0 && policy < SCHED_NUM, EINVAL);

	if (param)
	{
		param = U2K_GET_ADR(param, kthread_get_process(NULL));
		ASSERT_ERRNO_AND_EXIT(
			param->sched_priority >= THREAD_MIN_PRIO &&
			param->sched_priority <= THREAD_MAX_PRIO, EINVAL);
	}

	return kthread_setschedparam(kthread, policy, param);
}

/*! Set and get current thread error status */
int sys__set_errno(void *p)
{
	kthread_set_errno(NULL, *((int *) p));
	return 0;
}
int sys__get_errno(void *p)
{
	return kthread_get_errno(NULL);
}

int sys__get_errno_ptr(void *p)
{
	int **errno;

	errno = U2K_GET_ADR(*((int **) p), kthread_get_process(NULL));

	if (errno)
		*errno = K2U_GET_ADR(	kthread_get_errno_ptr(NULL),
					kthread_get_process(NULL));

	return errno != NULL;
}

/*! Mutex ------------------------------------------------------------------- */

/*!
 * Initialize mutex object
 * \param mutex Mutex descriptor (user level descriptor)
 * \param mutexattr Mutex parameters
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_mutex_init(void *p)
{
	pthread_mutex_t *mutex;
	/* pthread_mutexattr_t *mutexattr; not implemented */

	kprocess_t *proc;
	kpthread_mutex_t *kmutex;
	kobject_t *kobj;

	mutex = *((pthread_mutex_t **) p); /* p += sizeof(pthread_mutex_t *);
	mutexattr = *((pthread_mutexattr_t *) p); */

	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	proc = kthread_get_process(NULL);
	mutex = U2K_GET_ADR(mutex, proc);
	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	kobj = kmalloc_kobject(proc, sizeof(kpthread_mutex_t));
	ASSERT_ERRNO_AND_EXIT(kobj, ENOMEM);
	kmutex = kobj->kobject;

	kmutex->id = k_new_id();
	kmutex->owner = NULL;
	kmutex->flags = 0;
	kmutex->ref_cnt = 1;
	kthreadq_init(&kmutex->queue);

	mutex->ptr = kobj;
	mutex->id = kmutex->id;

	EXIT2(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Destroy mutex object
 * \param mutex Mutex descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_mutex_destroy(void *p)
{
	pthread_mutex_t *mutex;

	kprocess_t *proc;
	kpthread_mutex_t *kmutex;
	kobject_t *kobj;

	mutex = *((pthread_mutex_t **) p);
	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	proc = kthread_get_process(NULL);
	mutex = U2K_GET_ADR(mutex, proc);
	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	kobj = mutex->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
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
		EXIT2(EBUSY, EXIT_FAILURE);

	kfree_kobject(proc, kobj);

	mutex->ptr = NULL;
	mutex->id = 0;

	EXIT2(EXIT_SUCCESS, EXIT_SUCCESS);
}

static int mutex_lock(kpthread_mutex_t *kmutex, kthread_t *kthread);

/*!
 * Lock mutex object
 * \param mutex Mutex descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_mutex_lock(void *p)
{
	pthread_mutex_t *mutex;

	kprocess_t *proc;
	kpthread_mutex_t *kmutex;
	kobject_t *kobj;
	int retval = EXIT_SUCCESS;

	mutex = *((pthread_mutex_t **) p);
	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	proc = kthread_get_process(NULL);
	mutex = U2K_GET_ADR(mutex, proc);
	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	kobj = mutex->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EINVAL);
	kmutex = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(kmutex && kmutex->id == mutex->id, EINVAL);

	retval = mutex_lock(kmutex, kthread_get_active());

	if (retval == 1)
		kthreads_schedule();

	return retval != -1;
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
		kthread_enqueue(kthread, &kmutex->queue, 0, NULL, NULL);

		return 1;
	}
}

/*!
 * Unlock mutex object
 * \param mutex Mutex descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_mutex_unlock(void *p)
{
	pthread_mutex_t *mutex;

	kprocess_t *proc;
	kpthread_mutex_t *kmutex;
	kobject_t *kobj;

	mutex = *((pthread_mutex_t **) p);
	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	proc = kthread_get_process(NULL);
	mutex = U2K_GET_ADR(mutex, proc);
	ASSERT_ERRNO_AND_EXIT(mutex, EINVAL);

	kobj = mutex->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
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

	return EXIT_SUCCESS;
}

/*! conditional variables --------------------------------------------------- */

/*!
 * Initialize conditional variable object
 * \param cond conditional variable descriptor (user level descriptor)
 * \param condattr conditional variable descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_init(void *p)
{
	pthread_cond_t *cond;
	/* pthread_condattr_t *condattr; not implemented */

	kprocess_t *proc;
	kpthread_cond_t *kcond;
	kobject_t *kobj;

	cond = *((pthread_cond_t **) p); /* p += sizeof(pthread_cond_t *);
	condattr = *((pthread_condattr_t *) p); */

	ASSERT_ERRNO_AND_EXIT(cond, EINVAL);

	proc = kthread_get_process(NULL);
	cond = U2K_GET_ADR(cond, proc);
	ASSERT_ERRNO_AND_EXIT(cond, EINVAL);

	kobj = kmalloc_kobject(proc, sizeof(kpthread_cond_t));
	ASSERT_ERRNO_AND_EXIT(kobj, ENOMEM);
	kcond = kobj->kobject;

	kcond->id = k_new_id();
	kcond->flags = 0;
	kcond->ref_cnt = 1;
	kthreadq_init(&kcond->queue);

	cond->ptr = kobj;
	cond->id = kcond->id;

	EXIT2(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Destroy conditional variable object
 * \param cond conditional variable descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_destroy(void *p)
{
	pthread_cond_t *cond;

	kprocess_t *proc;
	kpthread_cond_t *kcond;
	kobject_t *kobj;

	cond = *((pthread_cond_t **) p);
	ASSERT_ERRNO_AND_EXIT(cond, EINVAL);

	proc = kthread_get_process(NULL);
	cond = U2K_GET_ADR(cond, proc);
	ASSERT_ERRNO_AND_EXIT(cond, EINVAL);

	kobj = cond->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EINVAL);
	kcond = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(kcond && kcond->id == cond->id, EINVAL);

	kcond->ref_cnt--;

	/* additional cleanup here (e.g. if cond.var. is shared leave it) */
	if (kcond->ref_cnt)
		EXIT2(EBUSY, EXIT_FAILURE);

	kfree_kobject(proc, kobj);

	cond->ptr = NULL;
	cond->id = 0;

	EXIT2(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Wait on conditional variable
 * \param cond conditional variable descriptor (user level descriptor)
 * \param mutex Mutex descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_wait(void *p)
{
	pthread_cond_t *cond;
	pthread_mutex_t *mutex;

	kprocess_t *proc;
	kpthread_cond_t *kcond;
	kpthread_mutex_t *kmutex;
	kobject_t *kobj_cond, *kobj_mutex;
	int retval = EXIT_SUCCESS;

	cond = *((pthread_cond_t **) p); p += sizeof(pthread_cond_t *);
	mutex = *((pthread_mutex_t **) p);
	ASSERT_ERRNO_AND_EXIT(cond && mutex, EINVAL);

	proc = kthread_get_process(NULL);
	cond = U2K_GET_ADR(cond, proc);
	mutex = U2K_GET_ADR(mutex, proc);
	ASSERT_ERRNO_AND_EXIT(cond && mutex, EINVAL);

	kobj_cond = cond->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj_cond, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj_cond->list),
				EINVAL);
	kcond = kobj_cond->kobject;
	ASSERT_ERRNO_AND_EXIT(kcond && kcond->id == cond->id, EINVAL);

	kobj_mutex = mutex->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj_mutex, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj_mutex->list),
				EINVAL);
	kmutex = kobj_mutex->kobject;
	ASSERT_ERRNO_AND_EXIT(kmutex && kmutex->id == mutex->id, EINVAL);

	ASSERT_ERRNO_AND_EXIT(kmutex->owner == kthread_get_active(), EPERM);

	SET_ERRNO(EXIT_SUCCESS);

	/* move thread in conditional variable queue */
	kthread_enqueue(NULL, &kcond->queue, 0, NULL, NULL);

	/* save reference to mutex object */
	kthread_set_private_param(NULL, kobj_mutex);

	/* release mutex */
	kmutex->owner = kthreadq_get(&kmutex->queue);
	if (kmutex->owner)
		kthreadq_release(&kmutex->queue);

	kthreads_schedule();

	return retval;
}

static int cond_release(void *p, int release_all);

/*!
 * Restart thread waiting on conditional variable
 * \param cond conditional variable descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_signal(void *p)
{
	return cond_release(p, FALSE);
}

/*!
 * Restart all threads waiting on conditional variable
 * \param cond conditional variable descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_cond_broadcast(void *p)
{
	return cond_release(p, TRUE);
}

static int cond_release(void *p, int release_all)
{
	pthread_cond_t *cond;

	kprocess_t *proc;
	kpthread_cond_t *kcond;
	kpthread_mutex_t *kmutex;
	kobject_t *kobj_cond, *kobj_mutex;
	kthread_t *kthread;
	int released = 0;

	cond = *((pthread_cond_t **) p);
	ASSERT_ERRNO_AND_EXIT(cond, EINVAL);

	proc = kthread_get_process(NULL);
	cond = U2K_GET_ADR(cond, proc);
	ASSERT_ERRNO_AND_EXIT(cond, EINVAL);

	kobj_cond = cond->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj_cond, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj_cond->list),
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

	return EXIT_SUCCESS;
}


/*! Semaphore --------------------------------------------------------------- */

/*!
 * Initialize semaphore object
 * \param sem Semaphore descriptor (user level descriptor)
 * \param pshared Shall semaphore object be shared between processes
 * \param value Initial semaphore value
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sem_init(void *p)
{
	sem_t *sem;
	int pshared;
	int value;

	kprocess_t *proc;
	ksem_t *ksem;
	kobject_t *kobj;

	sem =		*((sem_t **) p);	p += sizeof(sem_t *);
	pshared =	*((int *) p);		p += sizeof(int);
	value =		*((uint *) p);

	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	proc = kthread_get_process(NULL);
	sem = U2K_GET_ADR(sem, proc);
	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	kobj = kmalloc_kobject(proc, sizeof(ksem_t));
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

	EXIT2(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Destroy semaphore object
 * \param sem Semaphore descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sem_destroy(void *p)
{
	sem_t *sem;

	kprocess_t *proc;
	ksem_t *ksem;
	kobject_t *kobj;

	sem = *((sem_t **) p);

	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	proc = kthread_get_process(NULL);
	sem = U2K_GET_ADR(sem, proc);
	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	kobj = sem->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EINVAL);
	ksem = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ksem && ksem->id == sem->id, EINVAL);

	ASSERT_ERRNO_AND_EXIT(kthreadq_get(&ksem->queue) == NULL, ENOTEMPTY);

	ksem->ref_cnt--;

	/* additional cleanup here (e.g. if semaphore is shared leave it) */
	if (ksem->ref_cnt)
		EXIT2(EBUSY, EXIT_FAILURE);

	kfree_kobject(proc, kobj);

	sem->ptr = NULL;
	sem->id = 0;

	EXIT2(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Decrement (lock) semaphore value by 1 (if not 0 when thread is blocked)
 * \param sem Semaphore descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sem_wait(void *p)
{
	sem_t *sem;

	kprocess_t *proc;
	ksem_t *ksem;
	kobject_t *kobj;
	kthread_t *kthread;

	sem = *((sem_t **) p);
	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	proc = kthread_get_process(NULL);
	kthread = kthread_get_active();
	sem = U2K_GET_ADR(sem, proc);
	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	kobj = sem->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EINVAL);
	ksem = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ksem && ksem->id == sem->id, EINVAL);

	kthread_set_errno(kthread, EXIT_SUCCESS);

	if (ksem->sem_value > 0)
	{
		ksem->sem_value--;
		ksem->last_lock = kthread;
	}
	else {
		kthread_enqueue(kthread, &ksem->queue, 1, NULL, NULL);
		kthreads_schedule();
	}

	return EXIT_SUCCESS;
}

/*!
 * Increment (lock) semaphore value by 1 (or unblock one thread that is blocked)
 * \param sem Semaphore descriptor (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sem_post(void *p)
{
	sem_t *sem;

	kprocess_t *proc;
	ksem_t *ksem;
	kobject_t *kobj;
	kthread_t *kthread, *released;

	sem = *((sem_t **) p);

	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	proc = kthread_get_process(NULL);
	kthread = kthread_get_active();
	sem = U2K_GET_ADR(sem, proc);
	ASSERT_ERRNO_AND_EXIT(sem, EINVAL);

	kobj = sem->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EINVAL);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EINVAL);
	ksem = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(ksem && ksem->id == sem->id, EINVAL);

	kthread_set_errno(kthread, EXIT_SUCCESS);

	released = kthreadq_get(&ksem->queue); /* first to release */

	if (!released || ksem->sem_value < 0)
	{
		/* if initial semaphore value (set by sem_init) was negative,
		 * semaphore will not release threads until until its value
		 * reaches zero (small extension of POSIX semaphore) */
		ksem->sem_value++;
	}
	else {
		kthreadq_release(&ksem->queue);
		kthreads_schedule();
	}

	return EXIT_SUCCESS;
}

/*! Messages ---------------------------------------------------------------- */

/* list of message queues */
static list_t kmq_queue = LIST_T_NULL;

/*!
 * Open a message queue
 * \param name Queue name
 * \param oflag Opening flags
 * \param mode Permissions on created queue (only when O_CREAT is set)
 * \param attr Message queue attributes (only when O_CREAT is set)
 * \param mqdes Return queue descriptor address (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__mq_open(void *p)
{
	char *name;
	int oflag;
	/* mode_t mode;	not used in this implementation */
	mq_attr_t *attr;
	mqd_t *mqdes;

	kprocess_t *proc;
	kmq_queue_t *kq_queue;
	kobject_t *kobj;

	name =	*((char **) p);		p += sizeof(char *);
	oflag =	*((int *) p);			p += sizeof(int);
	/* mode = *((mode_t *) p); */		p += sizeof(mode_t);
	attr =	*((mq_attr_t **) p);		p += sizeof(mq_attr_t*);
	mqdes =	*((mqd_t **) p);

	ASSERT_ERRNO_AND_EXIT(name, EBADF);

	proc = kthread_get_process(NULL);
	name = U2K_GET_ADR(name, proc);
	mqdes = U2K_GET_ADR(mqdes, proc);
	ASSERT_ERRNO_AND_EXIT(name && mqdes, EBADF);
	ASSERT_ERRNO_AND_EXIT(strlen(name) < NAME_MAX, EBADF);


	kq_queue = list_get(&kmq_queue, FIRST);
	while (kq_queue &&
		strcmp(name, kq_queue->name))
		kq_queue = list_get_next(&kq_queue->list);

	if (	(kq_queue && ((oflag & O_CREAT) ||(oflag & O_EXCL)))
		||(!kq_queue && !(oflag & O_CREAT)))
	{
		mqdes->ptr = (void *) -1;
		mqdes->id = -1;
		EXIT2(EEXIST, EXIT_FAILURE);
	}

	if (!kq_queue && (oflag & O_CREAT))
	{
		kq_queue = kmalloc(sizeof(kmq_queue_t));

		if (attr)
		{
			attr = U2K_GET_ADR(attr, proc);
			ASSERT_ERRNO_AND_EXIT(attr, EINVAL);

			kq_queue->attr = *attr;
		}

		kq_queue->id = k_new_id();
		kq_queue->attr.mq_curmsgs = 0;

		kq_queue->name = kmalloc(strlen(name) + 1);
		strcpy(kq_queue->name, name);

		kq_queue->ref_cnt = 0;

		list_init(&kq_queue->msg_list);
		kthreadq_init(&kq_queue->recv_q);
		kthreadq_init(&kq_queue->send_q);

		list_append(&kmq_queue, kq_queue, &kq_queue->list);
	}

	kq_queue->ref_cnt++;

	kobj = kmalloc_kobject(proc, 0);
	kobj->kobject = kq_queue;
	kobj->flags = oflag;

	mqdes->ptr = kobj;
	mqdes->id = kq_queue->id;

	EXIT2(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Close a message queue
 * \param mqdes Queue descriptor address (user level descriptor)
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__mq_close(void *p)
{
	mqd_t *mqdes;

	kprocess_t *proc;
	kmq_queue_t *kq_queue;
	kobject_t *kobj;
	kmq_msg_t *kmq_msg;
	kthread_t *kthread;

	mqdes = *((mqd_t **) p);

	ASSERT_ERRNO_AND_EXIT(mqdes, EBADF);

	proc = kthread_get_process(NULL);
	mqdes = U2K_GET_ADR(mqdes, proc);
	ASSERT_ERRNO_AND_EXIT(mqdes, EBADF);

	kobj = mqdes->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EBADF);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EBADF);

	kq_queue = kobj->kobject;
	kq_queue = list_find_and_remove(&kmq_queue, &kq_queue->list);

	if (!kq_queue || kq_queue->id != mqdes->id)
		EXIT2(EBADF, EXIT_FAILURE);

	kq_queue->ref_cnt--;

	if (!kq_queue->ref_cnt)
	{
		/* remove messages */
		while ((kmq_msg = list_remove(&kq_queue->msg_list,FIRST,NULL)))
			kfree(kmq_msg);

		/* remove blocked threads */
		while ((kthread = kthreadq_remove(&kq_queue->send_q, NULL)))
		{
			kthread_move_to_ready(kthread, LAST);
			kthread_set_errno(kthread, EBADF);
			kthread_set_syscall_retval(kthread, EXIT_FAILURE);
		}
		while ((kthread = kthreadq_remove(&kq_queue->recv_q, NULL)))
		{
			kthread_move_to_ready(kthread, LAST);
			kthread_set_errno(kthread, EBADF);
			kthread_set_syscall_retval(kthread, EXIT_FAILURE);
		}

		list_remove(&kmq_queue, 0, &kq_queue->list);
		k_free_id(kq_queue->id);
		kfree(kq_queue->name);
		kfree(kq_queue);
	}

	/* remove kernel object descriptor */
	kfree_kobject(proc, kobj);

	EXIT2(EXIT_SUCCESS, EXIT_SUCCESS);
}

/* compare two messages by priority */
static int cmp_mq_msg(kmq_msg_t *m1, kmq_msg_t *m2)
{
	return m1->msg_prio - m2->msg_prio;
}

static int kmq_send(void *p, kthread_t *sender);
static int kmq_receive(void *p, kthread_t *sender);

/*!
 * Send a message to a message queue
 * \param mqdes Queue descriptor address (user level descriptor)
 * \param msg_ptr Message to be sent
 * \param msg_len Message size
 * \param msg_prio Message priority
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__mq_send(void *p)
{
	kthread_t *kthread;
	int retval;

	kthread = kthread_get_active();

	retval = kmq_send(p, kthread);

	if (retval == EXIT_SUCCESS)
	{
		kthread_set_errno(kthread, EXIT_SUCCESS);
	}
	else {
		kthread_set_errno(kthread, retval);
		retval = EXIT_FAILURE;
	}

	return retval;
}

static int kmq_send(void *p, kthread_t *sender)
{
	mqd_t *mqdes;
	char *msg_ptr;
	size_t msg_len;
	uint msg_prio;

	kprocess_t *proc = kthread_get_process(sender);
	kmq_queue_t *kq_queue;
	kobject_t *kobj;
	kmq_msg_t *kmq_msg;
	kthread_t *kthread;
	int retval;

	mqdes =		*((mqd_t **) p);	p += sizeof(mqd_t *);
	msg_ptr = 	*((char **) p);	p += sizeof(char *);
	msg_len = 	*((size_t *) p);	p += sizeof(size_t);
	msg_prio =	*((uint *) p);

	ASSERT_ERRNO_AND_EXIT(mqdes && msg_ptr, EINVAL);
	ASSERT_ERRNO_AND_EXIT(msg_prio <= MQ_PRIO_MAX, EINVAL);

	mqdes = U2K_GET_ADR(mqdes, proc);
	msg_ptr = U2K_GET_ADR(msg_ptr, proc);
	ASSERT_ERRNO_AND_EXIT(mqdes && msg_ptr, EINVAL);

	kobj = mqdes->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, EBADF);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				EBADF);

	kq_queue = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(kq_queue->id == mqdes->id, EBADF);
	ASSERT_ERRNO_AND_EXIT(list_find(&kmq_queue, &kq_queue->list),
				EBADF);

	if (kq_queue->attr.mq_curmsgs >= kq_queue->attr.mq_maxmsg)
	{
		if ((kobj->flags & O_NONBLOCK))
			return EAGAIN;

		/* block thread */
		kthread_enqueue(sender, &kq_queue->send_q, 1, NULL, NULL);
		kthreads_schedule();

		return EAGAIN;
	}

	if (msg_len > kq_queue->attr.mq_msgsize)
		return EMSGSIZE;

	kmq_msg = kmalloc(sizeof(kmq_msg_t) + msg_len);
	ASSERT_ERRNO_AND_EXIT(kmq_msg, ENOMEM);

	/* create message */
	kmq_msg->msg_size = msg_len;
	kmq_msg->msg_prio = msg_prio;
	memcpy(&kmq_msg->msg_data[0], msg_ptr, msg_len);

	list_sort_add(&kq_queue->msg_list, kmq_msg, &kmq_msg->list,
			(int (*)(void *, void *)) cmp_mq_msg);

	kq_queue->attr.mq_curmsgs++;

	/* is there a blocked receiver? */
	if ((kthread = kthreadq_remove(&kq_queue->recv_q, NULL)))
	{
		/* "save" sender thread */
		kthread_move_to_ready(sender, FIRST);
		kthread_set_errno(sender, EXIT_SUCCESS);
		kthread_set_syscall_retval(sender, EXIT_SUCCESS);

		/* unblock receiver */
		kthread_set_active(kthread); /* temporary */
		p = arch_syscall_get_params(kthread_get_context(kthread));

		retval = kmq_receive(p, kthread);

		if (retval >= 0)
		{
			kthread_set_errno(kthread, EXIT_SUCCESS);
			kthread_set_syscall_retval(kthread, retval);
		}
		else {
			kthread_set_errno(kthread, -retval);
			kthread_set_syscall_retval(kthread, EXIT_FAILURE);
		}

		kthreads_schedule();
	}

	return EXIT_SUCCESS;
}

/*!
 * Receive a message from a message queue
 * \param mqdes Queue descriptor address (user level descriptor)
 * \param msg_ptr Address to store message
 * \param msg_len Maximum message size
 * \param msg_prio Address to store message priority
 * \return length of selected message, -1 if error
 *
 * NOTE: since mq_receive returns size of received message, if error occurs
 *       returned error numbers are internally negated (only for this function!)
 */
int sys__mq_receive(void *p)
{
	kthread_t *kthread;
	int retval;

	kthread = kthread_get_active();

	retval = kmq_receive(p, kthread);

	if (retval >= 0)
	{
		kthread_set_errno(kthread, EXIT_SUCCESS);
	}
	else {
		kthread_set_errno(kthread, -retval);
		retval = EXIT_FAILURE;
	}

	return retval;
}

static int kmq_receive(void *p, kthread_t *receiver)
{
	mqd_t *mqdes;
	char *msg_ptr;
	size_t msg_len;
	uint *msg_prio;

	kprocess_t *proc = kthread_get_process(receiver);
	kmq_queue_t *kq_queue;
	kobject_t *kobj;
	kmq_msg_t *kmq_msg;
	kthread_t *kthread;
	int retval;

	mqdes =		*((mqd_t **) p);	p += sizeof(mqd_t *);
	msg_ptr = 	*((char **) p);	p += sizeof(char *);
	msg_len = 	*((size_t *) p);	p += sizeof(size_t);
	msg_prio =	*((uint **) p);

	ASSERT_ERRNO_AND_EXIT(mqdes && msg_ptr, -EINVAL);

	mqdes = U2K_GET_ADR(mqdes, proc);
	msg_ptr = U2K_GET_ADR(msg_ptr, proc);
	ASSERT_ERRNO_AND_EXIT(mqdes && msg_ptr, -EINVAL);

	kobj = mqdes->ptr;
	ASSERT_ERRNO_AND_EXIT(kobj, -EBADF);
	ASSERT_ERRNO_AND_EXIT(list_find(&proc->kobjects, &kobj->list),
				-EBADF);

	kq_queue = kobj->kobject;
	ASSERT_ERRNO_AND_EXIT(kq_queue->id == mqdes->id, -EBADF);
	ASSERT_ERRNO_AND_EXIT(list_find(&kmq_queue, &kq_queue->list),
				-EBADF);

	if (kq_queue->attr.mq_curmsgs == 0)
	{
		if ((kobj->flags & O_NONBLOCK))
			return -EAGAIN;

		/* block thread */
		kthread_enqueue(receiver, &kq_queue->recv_q, 1, NULL, NULL);
		kthreads_schedule();

		return -EAGAIN;
	}

	if (msg_len < kq_queue->attr.mq_msgsize)
		return -EMSGSIZE;

	kmq_msg = list_remove(&kq_queue->msg_list, FIRST, NULL);

	memcpy(msg_ptr, &kmq_msg->msg_data[0], kmq_msg->msg_size);
	msg_len = kmq_msg->msg_size;
	if (msg_prio)
	{
		msg_prio = U2K_GET_ADR(msg_prio, proc);
		if (msg_prio)
			*msg_prio = kmq_msg->msg_prio;
	}

	kfree(kmq_msg);

	kq_queue->attr.mq_curmsgs--;

	/* is there a blocked sender? */
	if ((kthread = kthreadq_remove(&kq_queue->send_q, NULL)))
	{
		/* "save" receiver thread */
		kthread_move_to_ready(receiver, FIRST);
		kthread_set_errno(receiver, EXIT_SUCCESS);
		kthread_set_syscall_retval(receiver, msg_len);

		/* unblock sender */
		kthread_set_active(kthread); /* temporary */
		p = arch_syscall_get_params(kthread_get_context(kthread));

		retval = kmq_send(p, kthread);

		if (retval == EXIT_SUCCESS)
		{
			kthread_set_errno(kthread, EXIT_SUCCESS);
			kthread_set_syscall_retval(kthread, retval);
		}
		else {
			kthread_set_errno(kthread, retval);
			kthread_set_syscall_retval(kthread, EXIT_FAILURE);
		}

		kthreads_schedule();
	}

	return msg_len;
}
