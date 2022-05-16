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
