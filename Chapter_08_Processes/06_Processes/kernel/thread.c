/*! Thread management */
#define _K_THREAD_C_
#define _K_SCHED_
#include "thread.h"

#include "memory.h"
#include "device.h"
#include "sched.h"
#include <arch/processor.h>
#include <arch/interrupt.h>
#include <arch/syscall.h>
#include <types/bits.h>
#include <lib/list.h>
#include <lib/string.h>
#include <kernel/errno.h>

static list_t all_threads; /* all threads */

static kthread_t *active_thread = NULL; /* active thread */

kprocess_t kernel_proc; /* kernel process (currently only for idle thread) */
static list_t kprocs; /* list of all processes */

static void kthread_remove_descriptor(kthread_t *kthread);
/* idle thread */
static void idle_thread(void *param);


/*! initialize thread structures and create idle thread */
void kthreads_init()
{
	list_init(&all_threads);
	list_init(&kprocs);

	active_thread = NULL;
	ksched_init();

	/* initially create 'idle thread' */
	kernel_proc.proc = NULL;
	kernel_proc.smap = NULL; /* use kernel pool */
	kernel_proc.m.start = NULL;
	kernel_proc.m.size = (size_t) 0xffffffff;

	(void) kthread_create(idle_thread, NULL, 0, SCHED_FIFO, 0, NULL,
				0, &kernel_proc);

	kthreads_schedule();
}

/*!
 * Start program defined by 'prog_name' (loaded as module) as new process:
 * - initialize environment (stack area for threads, stdin, stdout) and start
 *   it's first thread
 * \param prog_name Program name (as given with module)
 * \param param Command line arguments for starting thread (if not NULL)
 * \param prio Priority for starting thread
 * \return Pointer to descriptor of created process
 */
kthread_t *kthread_start_process(char *prog_name, void *param, int prio)
{
	extern list_t kprogs;
	kprog_t *kprog;
	kprocess_t *kproc;
	process_t *proc;
	kthread_t *kthread;
	char **args = NULL, *arg, **kargs;
	size_t argsize = 0;
	int i, j;

	kprog = list_get(&kprogs, FIRST);
	while (kprog && strcmp(kprog->prog->name, prog_name))
		kprog = list_get_next(&kprog->list);

	if (!kprog)
		return NULL;

	/* create new process */
	kproc = kmalloc(sizeof(kprocess_t));
	ASSERT(kproc);

	strcpy(kproc->name, kprog->prog->name);
	kproc->heap_size = kprog->prog->heap_size;
	kproc->stack_size = kprog->prog->stack_size;
	kproc->thread_stack_size = kprog->prog->thread_stack;
	kproc->prio = kprog->prog->prio;

	kproc->m.size = kprog->m->size +
			kproc->heap_size + kproc->stack_size;

	kproc->m.start = kmalloc(kproc->m.size);
	if (!kproc->m.start)
	{
		LOG(WARN, "Not enough memory for creating a new process!(%d)\n",
		      kproc->m.size);
		kfree(kproc);
		return NULL;
	}
	kproc->m.type = MS_PROCESS;

	/* copy code and data (memory segment with program) */
	memcpy(kproc->m.start, kprog->m->start, kprog->m->size);

	kproc->proc = (void *) kproc->m.start;
	proc = kproc->proc;

	/* define heap and stack */
	kproc->heap = (void *) kproc->m.start + kprog->m->size;
	kproc->stack = kproc->heap + kproc->heap_size;
	memset(kproc->heap, 0, kproc->heap_size + kproc->stack_size);

	/* initialize bitmap for threads stack management */
	/* in stack area: kproc->smap_size thread stacks */
	kproc->smap_size = kproc->stack_size / kproc->thread_stack_size;
	ASSERT(kproc->smap_size > 0);

	/* required number of uints for mask */
	i = (kproc->smap_size + sizeof(uint)*8 - 1) /(sizeof(uint)*8);

	kproc->smap = kmalloc(i * sizeof(uint));
	memset(kproc->smap, 0, i * sizeof(uint));
	for (j = kproc->smap_size%(sizeof(uint)*8+1); j < sizeof(uint)*8; j++)
		kproc->smap[i-1] |= 1<<j;

	/* set addresses in process header to relative/logical addresses */
	proc->heap = (void *) kprog->m->size;
	proc->stack = proc->heap + proc->p.heap_size;

	kproc->thread_count = 0;

	if (!prio)
		prio = kproc->prio;

	if (!prio)
	{
		prio = THR_DEFAULT_PRIO;
		kproc->prio = prio;
	}

	list_init(&kproc->kobjects);

	kargs = param;
	if (kargs && kargs[0]) /* have arguments? */
	{
		/* copy command line arguments from kernel space to process;
		  (use process heap space for arguments) */

		for (i = 0; kargs[i]; i++) /* count arguments */
			;
		/* kargs[i] == NULL, last argument */
		argsize = ((size_t) kargs[i-1] + strlen(kargs[i-1]) + 1)
			  - (size_t) param;
		/* look in sys__posix_spawn for argument packing */

		if (argsize > 0)
		{
			args = kproc->heap; /* pointers at start */
			proc->heap += argsize; proc->p.heap_size -= argsize;

			/* parameter data follow last pointer */
			arg = (void *) args + (i + 1) * sizeof(void *);

			for (i = 0; kargs[i]; i++)
			{
				strcpy(arg, kargs[i]);
				args[i] = K2U_GET_ADR(arg, kproc);
				arg += strlen(arg) + 1;
			}
			args[i] = NULL;
			args = K2U_GET_ADR(args, kproc);
		}

		kfree(param); /* allocated in pthread.c */
	}
	kthread = kthread_create(kproc->proc->p.init, args, 0, SCHED_FIFO,
				   prio, NULL, 0, kproc);

	list_append(&kprocs, kproc, &kproc->list);

	kthreads_schedule();

	return kthread;
}

/*!
 * Create new thread
 * \param start_routine Starting function for new thread
 * \param arg Parameter sent to starting function
 * \param sched_policy Thread scheduling policy
 * \param sched_priority Thread priority
 * \param stackaddr Address of thread stack (if not NULL)
 * \param stacksize Stack size
 * \param proc Process descriptor thread belongs to
 * \return Pointer to descriptor of created kernel thread
 */
kthread_t *kthread_create(void *start_routine, void *arg, uint flags,
	int sched_policy, int sched_priority,
	void *stackaddr, size_t stacksize, kprocess_t *proc)
{
	ASSERT(proc);

	kthread_t *kthread;

	/* thread descriptor */
	kthread = kmalloc(sizeof(kthread_t));
	ASSERT(kthread);

	/* initialize thread descriptor */
	kthread->id = k_new_id();

	kthread->proc = proc;
	kthread->proc->thread_count++;

	kthread->queue = NULL;
	kthreadq_init(&kthread->join_queue);

	kthread_create_new_state(kthread, start_routine, arg,
				   stackaddr, stacksize, FALSE);
	kthread->state.flags = flags;

	list_init(&kthread->states);

	/* connect signal mask in descriptor with state */
	kthread->sig_handling.mask = &kthread->state.sigmask;
	ksignal_thread_init(kthread);
	kthread->state.sig_int = 1;

	list_append(&all_threads, kthread, &kthread->all);

	kthread->sched_policy = sched_policy;
	if (sched_priority < 0)
		sched_priority = 0;
	if (sched_priority >= PRIO_LEVELS)
		sched_priority = PRIO_LEVELS - 1;
	kthread->sched_priority = sched_priority;

	kthread->ref_cnt = 1;
	kthread_move_to_ready(kthread, LAST);

	return kthread;
}

/*! create/insert state for signal handler and similar */
void kthread_create_new_state(kthread_t *kthread,
				void *start_func, void *param,
				void *stack, size_t stack_size,
				int save_old_state)
{
	ASSERT(kthread);
	kprocess_t *kproc = kthread_get_process(kthread);
	ASSERT(kproc);

	/* save old state if requested (put it at beginning of state list) */
	if (save_old_state)
	{
		kthread_state_t *state = kmalloc(sizeof(kthread_state_t));
		*state = kthread->state;
		list_prepend(&kthread->states, state, &state->list);
	}

	int stack_provided = FALSE;

	if (stack && stack_size)
	{
		stack_provided = TRUE;
	}
	else if (kproc->smap)
	{
		/* use process stack heap */
		stack_size = kproc->thread_stack_size;
		stack = kprocess_stack_alloc(kproc);
	}
	else {
		/* use kernel heap */
		if (!stack_size)
			stack_size = DEFAULT_THREAD_STACK_SIZE;

		stack = kmalloc(stack_size);
	}
	ASSERT(stack && stack_size);

	if (stack_provided)
	{
		kthread->state.stack = NULL; /* don't free it on exit */
		kthread->state.stack_size = 0;
	}
	else {
		kthread->state.stack = stack;
		kthread->state.stack_size = stack_size;
	}

	/* reserve space for errno in user space */
	stack_size -= sizeof(int);
	kthread->state.errno = kthread->state.stack + stack_size;

	arch_create_thread_context(&kthread->state.context, start_func, param,
				     kproc->proc->p.exit, stack, stack_size, kproc);

	*kthread->state.errno = 0;
	kthread->state.exit_status = NULL;
	kthread->state.pparam = NULL;

	list_init(&kthread->state.cleanup);
}

/*! restore previously saved state (last saved) */
int kthread_restore_state(kthread_t *kthread)
{
	ASSERT(kthread);

	kthread_state_t *state;
	/* perform cleanup of current state */
	kthread_state_cleanup_t *iter;
	while ((iter = list_remove(&kthread->state.cleanup, FIRST, NULL)))
	{
		iter->cleanup(iter->param1, iter->param2, iter->param3);
		kfree(iter);
	}

	/* release thread stack */
	if (kthread->state.stack)
	{
		if (kthread->proc->smap && kthread->state.stack)
			kprocess_stack_free(kthread->proc,
					      kthread->state.stack);

		else if (kthread->state.stack) /* kernel level thread */
			kfree(kthread->state.stack);
	}

	int retval = FALSE;

	/* overwrite state with first from list (if not empty) */
	state = list_remove(&kthread->states, FIRST, NULL);
	if (state)
	{
		kthread->state = *state;
		kfree(state);
		retval = TRUE;
	}

	return retval;
}

/*! Suspend thread (kthreads_schedule must follow this call) */
int kthread_suspend(kthread_t *kthread,void *wakeup_action,void *param)
{
	if (!kthread)
		kthread = active_thread;

	ASSERT(kthread->state.state == THR_STATE_ACTIVE ||
		 kthread->state.state == THR_STATE_READY);

	if (kthread->state.state == THR_STATE_READY)
		kthread_remove_from_ready(kthread);

	kthread->state.state = THR_STATE_SUSPENDED;
	kthread_set_signal_interrupt_handler(kthread, wakeup_action, param);

	return 0;
}

/*! Set signal (or other) interrupt handler for suspended or blocked thread */
int kthread_set_signal_interrupt_handler(
	kthread_t *kthread, void *wakeup_action, void *param)
{
	if (!kthread)
		kthread = active_thread;
	ASSERT(kthread);

	kthread->state.cancel_suspend_handler = wakeup_action;
	kthread->state.cancel_suspend_param = param;

	return 0;
}

static void kthread_release_prematurely(kthread_t *kthread, void *param)
{
	if (!kthread)
		kthread = active_thread;
	ASSERT(kthread);

	if (kthread->state.state == THR_STATE_WAIT)
	{
		if (!kthreadq_remove(kthread->queue, kthread))
			ASSERT(FALSE);
	}
	else if (kthread->state.state == THR_STATE_READY)
	{
		if (!kthread_remove_from_ready(kthread))
			ASSERT(FALSE);
	}
}

/*! add cleanup functions for when thread terminates or finish special processing
 *  like signal handlers */
void kthread_add_cleanup(kthread_t *kthread, void *cleanup_function,
			   param_t param1, param_t param2, param_t param3)
{
	ASSERT(kthread);

	kthread_state_cleanup_t *cleanup;
	cleanup = kmalloc(sizeof(kthread_state_cleanup_t));

	cleanup->cleanup = cleanup_function;
	cleanup->param1 = param1;
	cleanup->param2 = param2;
	cleanup->param3 = param3;

	list_append(&kthread->state.cleanup, cleanup, &cleanup->list);
}

/*!
 * Cancel thread (or restore it to previous state)
 * \param kthread Thread descriptor
 */
int kthread_exit(kthread_t *kthread, void *exit_status, int force)
{
	kthread_state_t *prev;
	int restored = FALSE;
	kthread_t *released;
	kthread_q *q;
	void **p;
	int waited = 0;

	ASSERT(kthread);

	do {
		prev = list_get(&kthread->states, FIRST);
		if (prev)
			restored = kthread_restore_state(kthread);
	}
	while (prev && force); /* if cancel is called, destroy all states */

	if (restored && !force) /* restored to previous state */
	{
		if (kthread == active_thread)
			kthread->state.state = THR_STATE_ACTIVE;

		kthreads_schedule();

		return EXIT_SUCCESS;
	}

	if (kthread->state.state == THR_STATE_PASSIVE)
		return EXIT_SUCCESS; /* thread is already finished */

	if (kthread->state.state == THR_STATE_READY)
	{
		/* remove target 'thread' from its queue */
		if (!kthread_remove_from_ready(kthread))
			ASSERT(FALSE);
	}
	else if (kthread->state.state == THR_STATE_WAIT)
	{
		/* remove target 'thread' from its queue */
		if (!kthreadq_remove(kthread->queue, kthread))
			ASSERT(FALSE);
	}
	else if (kthread->state.state == THR_STATE_SUSPENDED)
	{
		/* cancellation routine */
		if (kthread->state.cancel_suspend_handler)
			kthread->state.cancel_suspend_handler(kthread,
				kthread->state.cancel_suspend_param);
	}
	else if (kthread->state.state == THR_STATE_ACTIVE)
	{
		if (kthread != active_thread)
			return ESRCH; /* thread descriptor corrupted ! */
	}
	else {
		return ESRCH; /* thread descriptor corrupted ! */
	}

	kthread->state.state = THR_STATE_PASSIVE;
	if (waited > 0 || (kthread->state.flags & PTHREAD_CREATE_DETACHED))
		kthread->ref_cnt--;
	kthread->state.exit_status = exit_status;
	kthread->proc->thread_count--;

	arch_destroy_thread_context(&kthread->state.context);

	kthread_restore_state(kthread);

	if (kthread->proc->thread_count == 0 && kthread->proc)
	{
		/* last (non-kernel) thread - remove process */

		kfree_process_kobjects(kthread->proc);

		kfree(kthread->proc->m.start);
#ifdef DEBUG
		ASSERT(kthread->proc ==
			list_find_and_remove(&kprocs, &kthread->proc->list));
#else
		(void) list_remove(&kprocs, 0, &kthread->proc->list);
#endif
		kfree(kthread->proc);
		kthread->proc = NULL;
	}


	q = &kthread->join_queue;

	while ((released = kthreadq_remove(q, NULL)) != NULL)
	{
		/* save exit status to all waiting threads */
		p = kthread_get_private_param(released);
		if (p)
			*p = exit_status;

		kthread_move_to_ready(released, LAST);
		kthread->ref_cnt--;
	}

	if (!kthread->ref_cnt)
		kthread_remove_descriptor(kthread);

	if (kthread == active_thread)
	{
		active_thread = NULL;
		kthreads_schedule();
	}

	return EXIT_SUCCESS;
}

/*! Internal function for removing (freeing) thread descriptor */
static void kthread_remove_descriptor(kthread_t *kthread)
{
	ASSERT(kthread);

	k_free_id(kthread->id);
	kthread->id = 0;

#ifdef DEBUG
	ASSERT(kthread == list_find_and_remove(&all_threads, &kthread->all));
#else
	(void) list_remove(&all_threads, 0, &kthread->all);
#endif

	kfree(kthread);
}

/*!
 * Put given thread or active thread (when kthread == NULL) into queue of
 * "waited" thread
 */
void kthread_wait_thread(kthread_t *waiting, kthread_t *waited)
{
	ASSERT(waited);

	waited->ref_cnt++;
	kthread_enqueue(waiting, &waited->join_queue, 0, NULL, NULL);
}

/*! Get exit status of finished thread (and free descriptor) */
void kthread_collect_status(kthread_t *waited, void **retval)
{
	ASSERT(waited);

	if (retval)
		*retval = waited->state.exit_status;

	if (!waited->ref_cnt)
		kthread_remove_descriptor(waited);
}



/*! operations on thread queues (blocked threads) --------------------------- */

/*!
 * Put given thread or active thread (when kthread == NULL) into queue 'q_id'
 * - if kthread != NULL, thread must not be in any list and
 *   'kthreads_schedule' should follow this call before exiting from kernel!
 * \param kthread thread descriptor
 * \param q queue where to put thread
 * \param sig_int is thread interruptable while in queue?
 * \param wakeup_action if thread is interruptable on interrupt call this
 * \param param param for "wakeup_action"
 */
void kthread_enqueue(kthread_t *kthread, kthread_q *q, int sig_int,
		       void *wakeup_action, void *param)
{
	ASSERT((kthread || active_thread) && q);

	if (!kthread)
		kthread = active_thread;

	kthread->state.state = THR_STATE_WAIT;
	kthread->queue = q;
	kthread->state.sig_int = sig_int;

	if (sig_int && wakeup_action == NULL)
		wakeup_action = kthread_release_prematurely;
	kthread_set_signal_interrupt_handler(kthread, wakeup_action, param);

	kthreadq_append(kthread->queue, kthread);
}

/*!
 * Release single thread from given queue (if queue not empty)
 * \param q Queue
 * \return 1 if thread was released, 0 if queue was empty
 */
int kthreadq_release(kthread_q *q)
{
	kthread_t *kthread;

	ASSERT(q);

	kthread = kthreadq_remove(q, NULL);

	if (kthread)
	{
		kthread_move_to_ready(kthread, LAST);
		kthread->state.sig_int = 1;

		return 1;
	}
	else {
		return 0;
	}
}

/*!
 * Release all threads from given queue (if queue not empty)
 * \param q Queue
 * \return number of thread released, 0 if queue was empty
 */
int kthreadq_release_all(kthread_q *q)
{
	ASSERT(q);
	int cnt = 0;

	while (kthreadq_release(q))
		cnt++;

	return cnt;
}

/*! thread queue manipulation */
void kthreadq_init(kthread_q *q)
{
	ASSERT(q);
	list_init(&q->q);
}
void kthreadq_append(kthread_q *q, kthread_t *kthread)
{
	ASSERT(kthread && q);
	list_append(&q->q, kthread, &kthread->list);
}
void kthreadq_prepend(kthread_q *q, kthread_t *kthread)
{
	ASSERT(kthread && q);
	list_prepend(&q->q, kthread, &kthread->list);
}
kthread_t *kthreadq_remove(kthread_q *q, kthread_t *kthread)
{
	ASSERT(q);
	if (kthread)
		return list_find_and_remove(&q->q, &kthread->list);
	else
		return list_remove(&q->q, FIRST, NULL);
}
kthread_t *kthreadq_get(kthread_q *q)
{
	ASSERT(q);
	return list_get(&q->q, FIRST);
}
kthread_t *kthreadq_get_next(kthread_t *kthread)
{
	ASSERT(kthread);
	return list_get_next(&kthread->list);   /* kthread->queue->q.first->object */
}

/*! get thread scheduling policy */
int kthread_get_sched_policy(kthread_t *kthread)
{
	if (kthread)
		return kthread->sched_policy;
	else
		return active_thread->sched_policy;
}

int kthread_get_prio(kthread_t *kthread)
{
	if (kthread)
		return kthread->sched_priority;
	else
		return active_thread->sched_priority;
}
int kthread_set_prio(kthread_t *kthread, int prio)
{
	kthread_t *kthr = kthread;
	int old_prio;

	if (!kthr)
		kthr = active_thread;

	old_prio = kthr->sched_priority;

	/* change thread priority:
	(i)	if its active: change priority and move to ready
	(ii)	if its ready: remove from queue, change priority, put back
	(iii)	if its blocked: if queue is sorted by priority, same as(ii)
	*/
	switch(kthr->state.state)
	{
	case THR_STATE_ACTIVE:
		kthr->sched_priority = prio;
		kthread_move_to_ready(kthr, LAST);
		kthreads_schedule();
		break;

	case THR_STATE_READY:
		kthread_remove_from_ready(kthr);
		kthr->sched_priority = prio;
		kthread_move_to_ready(kthr, LAST);
		kthreads_schedule();
		break;

	case THR_STATE_WAIT: /* as now there is only FIFO queue */
		kthr->sched_priority = prio;
		break;

	case THR_STATE_PASSIVE: /* report error or just change priority? */
		kthr->sched_priority = prio;
		break;
	}

	return old_prio;
}

/*! Get-ers, Set-ers and misc ----------------------------------------------- */

int kthread_is_active(kthread_t *kthread)
{
	ASSERT(kthread);
	if (kthread->state.state == THR_STATE_ACTIVE)
		return TRUE;
	else
		return FALSE;
}

int kthread_is_ready(kthread_t *kthread)
{
	ASSERT(kthread);

	return	kthread->state.state == THR_STATE_ACTIVE ||
		kthread->state.state == THR_STATE_READY;
}

int kthread_is_alive(kthread_t *kthread)
{
	return	kthread->state.state != THR_STATE_PASSIVE &&
		kthread_check_kthread(kthread);
}

int kthread_is_passive(kthread_t *kthread)
{
	ASSERT(kthread);
	if (kthread->state.state == THR_STATE_PASSIVE)
		return TRUE;
	else
		return FALSE;
}

int kthread_is_suspended(kthread_t *kthread, void **func, void **param)
{
	if (!kthread)
		kthread = active_thread;

	if (func)
		*func = kthread->state.cancel_suspend_handler;
	if (param)
		*param = kthread->state.cancel_suspend_param;

	if (kthread->state.state == THR_STATE_SUSPENDED)
		return TRUE;
	else
		return FALSE;
}

/*! check if thread descriptor is valid, i.e. is in all thread list */
int kthread_check_kthread(kthread_t *kthread)
{
	return kthread && list_find(&all_threads, &kthread->all);
}

int kthread_get_id(kthread_t *kthread)
{
	if (kthread)
		return kthread->id;
	else
		return active_thread->id;
}

kthread_t *kthread_get_active()
{
	return active_thread;
}

void *kthread_get_context(kthread_t *kthread)
{
	if (kthread)
		return &kthread->state.context;
	else
		return &active_thread->state.context;
}

void *kthread_get_process(kthread_t *kthread)
{
	if (kthread)
		return kthread->proc;
	else
		return active_thread->proc;
}

/*! Get kernel thread descriptor from user thread descriptor */
kthread_t *kthread_get_descriptor(pthread_t *thread)
{
	kthread_t *kthread;

	if (thread && (kthread = thread->ptr) && thread->id == kthread->id &&
		kthread->state.state != THR_STATE_PASSIVE)
		return kthread;
	else
		return NULL;
}

void *kthread_get_sigparams(kthread_t *kthread)
{
	if (!kthread)
		kthread = active_thread;
	ASSERT(kthread);
	return &kthread->sig_handling;
}

int kthread_get_interruptable(kthread_t *kthread)
{
	if (!kthread)
		kthread = active_thread;
	ASSERT(kthread);
	return kthread->state.sig_int;
}

void kthread_set_active(kthread_t *kthread)
{
	ASSERT(kthread);
	active_thread = kthread;
	active_thread->state.state = THR_STATE_ACTIVE;
	active_thread->queue = NULL;
}
void kthread_mark_ready(kthread_t *kthread)
{
	ASSERT(kthread);
	kthread->state.state = THR_STATE_READY;
}
void kthread_set_queue(kthread_t *kthread, kthread_q *queue)
{
	ASSERT(kthread && queue);
	kthread->queue = queue;
}
kthread_q *kthread_get_queue(kthread_t *kthread)
{
	ASSERT(kthread);
	return kthread->queue;
}


/*! Temporary storage for blocked thread (save specific context before wait) */
void kthread_set_private_param(kthread_t *kthread, void *pparam)
{
	if (!kthread)
		kthread = active_thread;
	kthread->state.pparam = pparam;
}
void *kthread_get_private_param(kthread_t *kthread)
{
	if (!kthread)
		kthread = active_thread;
	return kthread->state.pparam;
}


/*! errno and return value */
void kthread_set_errno(kthread_t *kthread, int error_number)
{
	if (kthread)
		*kthread->state.errno = error_number;
	else
		*active_thread->state.errno = error_number;
}
int kthread_get_errno(kthread_t *kthread)
{
	if (kthread)
		return *kthread->state.errno;
	else
		return *active_thread->state.errno;
}
int *kthread_get_errno_ptr(kthread_t *kthread)
{
	if (kthread)
		return kthread->state.errno;
	else
		return active_thread->state.errno;
}
void kthread_set_syscall_retval(kthread_t *kthread, int ret_val)
{
	if (!kthread)
		kthread = active_thread;
	ASSERT(kthread);
	arch_syscall_set_retval(kthread_get_context(kthread), ret_val);
}


/*! display active & ready threads info on console */
int kthread_info()
{
	kthread_t *kthread;
	int i = 1;

	kprintf("Threads info\n");

	kprintf("[this]\tid=%d(desc. at %x) in process at %x, size=%d\n",
		  active_thread->id, active_thread, active_thread->proc->m.start,
		  active_thread->proc->m.size);

	kprintf("\tprio=%d, state=%d, exit_status=%x\n",
		  active_thread->sched_priority, active_thread->state.state,
		  active_thread->state.exit_status);

	kthread = list_get(&all_threads, FIRST);
	while (kthread)
	{
		kprintf("[%d]\tid=%d(desc. at %x) in process at %x, size=%d\n",
			 i++, kthread->id, kthread, kthread->proc->m.start,
			 kthread->proc->m.size);

		kprintf("\tprio=%d, state=%d, exit_status=%x\n",
			 kthread->sched_priority, kthread->state.state,
			 kthread->state.exit_status);

		kthread = list_get_next(&kthread->all);
	}

	return 0;
}

/*! Idle thread ------------------------------------------------------------- */
#include <api/syscall.h>

/*! Idle thread starting (and only) function */
static void idle_thread(void *param)
{
	while (1)
		user_mode_suspend();
}

/*! Change thread scheduling parameters ------------------------------------- */
int kthread_setschedparam(kthread_t *kthread, int policy, sched_param_t *param)
{
	int sched_priority;

	ASSERT_ERRNO_AND_EXIT(kthread, EINVAL);
	ASSERT_ERRNO_AND_EXIT(kthread_is_alive(kthread), ESRCH);
	ASSERT_ERRNO_AND_EXIT(policy >= 0 && policy < SCHED_NUM, EINVAL);

	if (param)
	{
		ASSERT_ERRNO_AND_EXIT(
			param->sched_priority >= THREAD_MIN_PRIO &&
			param->sched_priority <= THREAD_MAX_PRIO, EINVAL);

		if (param->sched_priority)
			sched_priority = param->sched_priority;
		else
			sched_priority = kthread->sched_priority;
	}
	else {
		sched_priority = kthread->sched_priority;
	}

	/* change in priority? */
	if (kthread->sched_priority != sched_priority)
		kthread_set_prio(kthread, sched_priority);

	return EXIT_SUCCESS;
}
