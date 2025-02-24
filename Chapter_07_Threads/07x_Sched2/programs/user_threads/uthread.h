/*! Simple user managed threads */

#pragma once

#ifndef USER_THREAD_C

typedef void uthread_t;

#else

#include <lib/list.h>
#include <arch/context.h>

typedef struct _uthread_t_
{
	int id;
	context_t context;
	void *stack;
	list_h list;
}
uthread_t;

#endif /* USER_THREAD_C */

void uthreads_init();
uthread_t *create_uthread(void (func)(void *), void *param);
void uthread_exit();
void uthread_yield();
