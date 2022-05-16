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
