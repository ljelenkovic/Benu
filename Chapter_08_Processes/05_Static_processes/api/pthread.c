/*! POSIX threads and related operations (synchronization and communication) */

#include <api/pthread.h>

#include <api/stdio.h>
#include <api/syscall.h>
#include <api/errno.h>
#include <types/basic.h>

/*! Thread creation/exit/wait/cancel ---------------------------------------- */

int pthread_create(pthread_t *thread, pthread_attr_t *attr,
		     void *(*start_routine)(void *), void *arg)
{
	ASSERT_ERRNO_AND_RETURN(start_routine, EINVAL);

	return syscall(PTHREAD_CREATE, thread, attr, start_routine, arg);
}

void pthread_exit(void *retval)
{
	syscall(PTHREAD_EXIT, retval);
}

int pthread_join(pthread_t thread, void **retval)
{
	return syscall(PTHREAD_JOIN, &thread, retval);
}

pthread_t pthread_self(void)
{
	pthread_t self;

	self.ptr = NULL;
	self.id = 0;

	syscall(PTHREAD_SELF, &self);

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

	return syscall(PTHREAD_SETSCHEDPARAM, &thread, policy, param);
}

/*! Get thread scheduling parameters */
int pthread_getschedparam(pthread_t thread, int *policy,
			    struct sched_param *param)
{
	set_errno(ENOTSUP);

	return EXIT_FAILURE;
}

/*! Start program */
int posix_spawn(pid_t *pid, char *path, void *file_actions,
		  void *attrp, char *argv[], char *envp[])
{
	ASSERT_ERRNO_AND_RETURN(path, EINVAL);
	return syscall(POSIX_SPAWN, pid, path, file_actions, attrp, argv, envp);
}


/*! Mutex */
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
	ASSERT_ERRNO_AND_RETURN(mutex, EINVAL);
	return syscall(PTHREAD_MUTEX_INIT, mutex, attr);
}
int pthread_mutex_destroy(pthread_mutex_t * mutex)
{
	ASSERT_ERRNO_AND_RETURN(mutex, EINVAL);
	return syscall(PTHREAD_MUTEX_DESTROY, mutex);
}
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	ASSERT_ERRNO_AND_RETURN(mutex, EINVAL);
	return syscall(PTHREAD_MUTEX_LOCK, mutex);
}
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	ASSERT_ERRNO_AND_RETURN(mutex, EINVAL);
	return syscall(PTHREAD_MUTEX_UNLOCK, mutex);
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
	return syscall(PTHREAD_COND_INIT, cond);
}
int pthread_cond_destroy(pthread_cond_t *cond)
{
	ASSERT_ERRNO_AND_RETURN(cond, EINVAL);
	return syscall(PTHREAD_COND_DESTROY, cond);
}
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	ASSERT_ERRNO_AND_RETURN(cond && mutex, EINVAL);
	return syscall(PTHREAD_COND_WAIT, cond, mutex);
}
int pthread_cond_signal(pthread_cond_t *cond)
{
	ASSERT_ERRNO_AND_RETURN(cond, EINVAL);
	return syscall(PTHREAD_COND_SIGNAL, cond);
}
int pthread_cond_broadcast(pthread_cond_t *cond)
{
	ASSERT_ERRNO_AND_RETURN(cond, EINVAL);
	return syscall(PTHREAD_COND_BROADCAST, cond);
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
	return syscall(SEM_INIT, sem, pshared, value);
}
int sem_destroy(sem_t *sem)
{
	ASSERT_ERRNO_AND_RETURN(sem, EINVAL);
	return syscall(SEM_DESTROY, sem);
}
int sem_post(sem_t *sem)
{
	ASSERT_ERRNO_AND_RETURN(sem, EINVAL);
	return syscall(SEM_POST, sem);
}
int sem_wait(sem_t *sem)
{
	ASSERT_ERRNO_AND_RETURN(sem, EINVAL);
	return syscall(SEM_WAIT, sem);
}

/*! Message queue */
mqd_t mq_open(char *name, int oflag, mode_t mode, struct mq_attr *attr)
{
	mqd_t mqdes;
	if (!name)
	{
		mqdes.id = -1;
		mqdes.ptr = (void *) -1;
		set_errno(EINVAL);
	}
	else {
		syscall(MQ_OPEN, name, oflag, mode, attr, &mqdes);
	}
	return mqdes;
}
int mq_close(mqd_t mqdes)
{
	ASSERT_ERRNO_AND_RETURN(mqdes.id != -1 && mqdes.ptr != (void *) -1,
				  EINVAL);
	return syscall(MQ_CLOSE, &mqdes);
}
int mq_send(mqd_t mqdes, char *msg_ptr, size_t msg_len, uint msg_prio)
{
	ASSERT_ERRNO_AND_RETURN(mqdes.id != -1 && mqdes.ptr != (void *) -1,
				  EINVAL);
	ASSERT_ERRNO_AND_RETURN(msg_ptr, EINVAL);
	return syscall(MQ_SEND, &mqdes, msg_ptr, msg_len, msg_prio);
}
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, uint *msg_prio)
{
	ASSERT_ERRNO_AND_RETURN(mqdes.id != -1 && mqdes.ptr != (void *) -1,
				  EINVAL);
	ASSERT_ERRNO_AND_RETURN(msg_ptr, EINVAL);
	return syscall(MQ_RECEIVE, &mqdes, msg_ptr, msg_len, msg_prio);
}
