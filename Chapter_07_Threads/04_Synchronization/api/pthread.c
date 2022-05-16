/*! POSIX threads and related operations (synchronization and communication) */

#include <api/pthread.h>

#include <api/stdio.h>
#include <kernel/pthread.h>
#include <api/errno.h>
#include <types/basic.h>

/*! Thread creation/exit/wait/cancel ---------------------------------------- */

int pthread_create(pthread_t *thread, pthread_attr_t *attr,
		     void *(*start_routine)(void *), void *arg)
{
	ASSERT_ERRNO_AND_RETURN(start_routine, EINVAL);

	return sys__pthread_create(thread, attr, start_routine, arg);
}

void pthread_exit(void *retval)
{
	sys__pthread_exit(retval);
}

int pthread_join(pthread_t thread, void **retval)
{
	return sys__pthread_join(&thread, retval);
}

pthread_t pthread_self(void)
{
	pthread_t self;

	self.ptr = NULL;
	self.id = 0;

	sys__pthread_self(&self);

	return self;
}

int pthread_attr_init(pthread_attr_t *attr)
{
	ASSERT_ERRNO_AND_RETURN(attr, EINVAL);

	attr->flags =	PTHREAD_INHERIT_SCHED | PTHREAD_CREATE_JOINABLE |
			PTHREAD_SCOPE_SYSTEM;

	attr->sched_policy = SCHED_FIFO;
	attr->sched_params.sched_priority = THREAD_DEF_PRIO;

	attr->stackaddr = NULL;
	attr->stacksize = 0;

	return EXIT_SUCCESS;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
	ASSERT_ERRNO_AND_RETURN(attr, EINVAL);
	return EXIT_SUCCESS;
}

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
	ASSERT_ERRNO_AND_RETURN(attr, EINVAL);
	ASSERT_ERRNO_AND_RETURN(policy >= 0 && policy < SCHED_NUM, EINVAL);

	attr->sched_policy = policy;

	return EXIT_SUCCESS;
}

int pthread_attr_setschedparam(pthread_attr_t *attr,
				 struct sched_param *param)
{
	ASSERT_ERRNO_AND_RETURN(attr && param, EINVAL);
	ASSERT_ERRNO_AND_RETURN(param->sched_priority >= THREAD_MIN_PRIO &&
		param->sched_priority <= THREAD_MAX_PRIO, EINVAL);

	attr->sched_params = *param;

	return EXIT_SUCCESS;
}

/*! Set thread scheduling parameters */
int pthread_setschedparam(pthread_t thread, int policy,
			    struct sched_param *param)
{
	ASSERT_ERRNO_AND_RETURN(policy >= 0 && policy < SCHED_NUM, EINVAL);

	if (param)
		ASSERT_ERRNO_AND_RETURN(
			param->sched_priority >= THREAD_MIN_PRIO &&
			param->sched_priority <= THREAD_MAX_PRIO, EINVAL);

	return sys__pthread_setschedparam(&thread, policy, param);
}

/*! Get thread scheduling parameters */
int pthread_getschedparam(pthread_t thread, int *policy,
			    struct sched_param *param)
{
	set_errno(ENOTSUP);

	return EXIT_FAILURE;
}


/*! Mutex */
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
	ASSERT_ERRNO_AND_RETURN(mutex, EINVAL);
	return sys__pthread_mutex_init(mutex, attr);
}
int pthread_mutex_destroy(pthread_mutex_t * mutex)
{
	ASSERT_ERRNO_AND_RETURN(mutex, EINVAL);
	return sys__pthread_mutex_destroy(mutex);
}
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	ASSERT_ERRNO_AND_RETURN(mutex, EINVAL);
	return sys__pthread_mutex_lock(mutex);
}
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	ASSERT_ERRNO_AND_RETURN(mutex, EINVAL);
	return sys__pthread_mutex_unlock(mutex);
}
int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	ASSERT_ERRNO_AND_RETURN(attr, EINVAL);
	*((uint*) attr) = 0;
	return EXIT_SUCCESS;
}
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	ASSERT_ERRNO_AND_RETURN(attr, EINVAL);
	return EXIT_SUCCESS;
}

/*! Condition variable */
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr)
{
	ASSERT_ERRNO_AND_RETURN(cond, EINVAL);
	return sys__pthread_cond_init(cond, attr);
}
int pthread_cond_destroy(pthread_cond_t *cond)
{
	ASSERT_ERRNO_AND_RETURN(cond, EINVAL);
	return sys__pthread_cond_destroy(cond);
}
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	ASSERT_ERRNO_AND_RETURN(cond && mutex, EINVAL);
	return sys__pthread_cond_wait(cond, mutex);
}
int pthread_cond_signal(pthread_cond_t *cond)
{
	ASSERT_ERRNO_AND_RETURN(cond, EINVAL);
	return sys__pthread_cond_signal(cond);
}
int pthread_cond_broadcast(pthread_cond_t *cond)
{
	ASSERT_ERRNO_AND_RETURN(cond, EINVAL);
	return sys__pthread_cond_broadcast(cond);
}

int pthread_condattr_init(pthread_condattr_t *attr)
{
	ASSERT_ERRNO_AND_RETURN(attr, EINVAL);
	return EXIT_SUCCESS;
}
int pthread_condattr_destroy(pthread_condattr_t *attr)
{
	ASSERT_ERRNO_AND_RETURN(attr, EINVAL);
	return EXIT_SUCCESS;
}

/*! Semaphore */
int sem_init(sem_t *sem, int pshared, int value)
{
	ASSERT_ERRNO_AND_RETURN(sem, EINVAL);
	return sys__sem_init(sem, pshared, value);
}
int sem_destroy(sem_t *sem)
{
	ASSERT_ERRNO_AND_RETURN(sem, EINVAL);
	return sys__sem_destroy(sem);
}
int sem_post(sem_t *sem)
{
	ASSERT_ERRNO_AND_RETURN(sem, EINVAL);
	return sys__sem_post(sem);
}
int sem_wait(sem_t *sem)
{
	ASSERT_ERRNO_AND_RETURN(sem, EINVAL);
	return sys__sem_wait(sem);
}
