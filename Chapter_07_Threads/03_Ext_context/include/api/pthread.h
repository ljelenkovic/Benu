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
