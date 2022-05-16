/*! POSIX threads and related operations (synchronization and communication) */
#pragma once

#include <types/pthread.h>

/*! POSIX thread interface */
int pthread_create(pthread_t *thread, pthread_attr_t *attr,
		     void *(*start_routine)(void *), void *arg);
void pthread_exit(void *retval);
int pthread_join(pthread_t thread, void **retval);
pthread_t pthread_self(void);

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);

/*! Scheduling parameters */
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_setschedparam(pthread_attr_t *attr,
				 struct sched_param *param);

int pthread_setschedparam(pthread_t thread, int policy,
			    struct sched_param *param);
int pthread_getschedparam(pthread_t thread, int *policy,
			    struct sched_param *param);

/*! Create process */
int posix_spawn(pid_t *pid, char *path, void *file_actions,
		  void *attrp, char *argv[], char *envp[]);

/*! Mutex */
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t * mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

/*! Condition variable */
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_setschedparam(pthread_attr_t *attr,
				 struct sched_param *param);
/*! Semaphore */
int sem_init(sem_t *sem, int pshared, int value);
int sem_destroy(sem_t *sem);
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);

/*! Message queue */
mqd_t mq_open(char *name, int oflag, mode_t mode, struct mq_attr *attr);
int mq_close(mqd_t mqdes);
int mq_send(mqd_t mqdes, char *msg_ptr, size_t msg_len, uint msg_prio);
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, uint *msg_prio);
