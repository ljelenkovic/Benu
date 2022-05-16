/*! POSIX threads kernel interface */
#pragma once

#include <types/pthread.h>

/*! interface for threads (via software interrupt) -------------------------- */
int sys__pthread_create(pthread_t *thread, pthread_attr_t *attr,
			  void *(*start_routine)(void *), void *arg);
int sys__pthread_exit(void *retval);
int sys__pthread_join(pthread_t *thread, void **retval);
int sys__pthread_self(pthread_t *thread);

int sys__pthread_setschedparam(pthread_t *thread, int policy,
				 sched_param_t *param);

int sys__pthread_mutex_init(pthread_mutex_t *mutex,
			      pthread_mutexattr_t *mutexattr);
int sys__pthread_mutex_destroy(pthread_mutex_t *mutex);
int sys__pthread_mutex_lock(pthread_mutex_t *mutex);
int sys__pthread_mutex_unlock(pthread_mutex_t *mutex);

int sys__pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *condattr);
int sys__pthread_cond_destroy(pthread_cond_t *cond);
int sys__pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int sys__pthread_cond_signal(pthread_cond_t *cond);
int sys__pthread_cond_broadcast(pthread_cond_t *cond);

int sys__sem_init(sem_t *sem, int pshared, int value);
int sys__sem_destroy(sem_t *sem);
int sys__sem_wait(sem_t *sem);
int sys__sem_post(sem_t *sem);
