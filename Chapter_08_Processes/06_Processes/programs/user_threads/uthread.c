/*! Simple user threads */

#define USER_THREAD_C

#include "uthread.h"

#include <malloc.h>

static int next_id;
static list_t active;
static list_t ready;

static uthread_t main_thread;

void uthreads_init()
{
	next_id = 1;

	list_init(&active);
	list_append(&active, &main_thread, &main_thread.list);
	list_init(&ready);
}

uthread_t *create_uthread(void (func)(void *), void *param)
{
	uthread_t *thread;

	thread = malloc(sizeof(uthread_t));
	thread->stack = malloc(THREAD_STACK_SIZE);

	thread->id = next_id;

	arch_create_uthread_context(&thread->context, func, param,
			uthread_exit, thread->stack, THREAD_STACK_SIZE);

	next_id++;
	list_append(&ready, thread, &thread->list);

	return thread;
}

void uthread_exit()
{
	uthread_t *cur_thread, *new_thread, tmp;

	/* active thread is exiting */
	cur_thread = list_remove(&active, FIRST, NULL);

	free(cur_thread->stack);
	/* freeing it, but using it still - till the end of this function
	  (parameters are on stack!) */

	tmp = *cur_thread;
	free(cur_thread);

	/* pick next ready thread */
	new_thread = list_remove(&ready, FIRST, NULL);
	/* make it active */
	list_append(&active, new_thread, &new_thread->list);
	/* switch to it */
	arch_switch_to_uthread(&tmp.context, &new_thread->context);
}

void uthread_yield()
{
	uthread_t *cur_thread, *new_thread;

	/* remove current thread from 'active' and put it as last in 'ready' */
	cur_thread = list_remove(&active, FIRST, NULL);
	list_append(&ready, cur_thread, &cur_thread->list);

	/* remove first thread from 'ready' and put it in 'active' */
	new_thread = list_remove(&ready, FIRST, NULL);
	list_append(&active, new_thread, &new_thread->list);

	/* switch to new active thread */
	arch_switch_to_uthread(&cur_thread->context, &new_thread->context);
}
