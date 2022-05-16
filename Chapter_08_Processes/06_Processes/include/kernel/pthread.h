/*! POSIX threads kernel interface */
#pragma once

#include <types/pthread.h>

/*! interface for threads (via software interrupt) -------------------------- */
int sys__pthread_create(void *p);
int sys__pthread_exit(void *p);
int sys__pthread_join(void *p);
int sys__pthread_self(void *p);

int sys__pthread_setschedparam(void *p);

int sys__posix_spawn(void *p);

int sys__pthread_mutex_init(void *p);
int sys__pthread_mutex_destroy(void *p);
int sys__pthread_mutex_lock(void *p);
int sys__pthread_mutex_unlock(void *p);
int sys__pthread_cond_init(void *p);
int sys__pthread_cond_destroy(void *p);
int sys__pthread_cond_wait(void *p);
int sys__pthread_cond_signal(void *p);
int sys__pthread_cond_broadcast(void *p);

int sys__sem_init(void *p);
int sys__sem_destroy(void *p);
int sys__sem_wait(void *p);
int sys__sem_post(void *p);

int sys__mq_open(void *p);
int sys__mq_close(void *p);
int sys__mq_send(void *p);
int sys__mq_receive(void *p);
