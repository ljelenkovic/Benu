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
kprocess_t user_proc;   /* user process descriptor */

static void kthread_remove_descriptor ( kthread_t *kthread );
/* idle thread */
static void idle_thread ( void *param );


/*! initialize thread structures and create idle thread */
void kthreads_init ()
{
	extern kprog_t prog;
	int prio;

	list_init ( &all_threads );

	active_thread = NULL;
	ksched_init ();

	/* initially create 'idle thread' */
	kernel_proc.prog = NULL;
	kernel_proc.stack_pool = NULL; /* use kernel pool */
	kernel_proc.m.start = NULL;
	kernel_proc.m.size = (size_t) 0xffffffff;

	(void) kthread_create ( idle_thread, NULL, 0, SCHED_FIFO, 0, NULL,
				NULL, 0, &kernel_proc );

	/* user_proc */
	user_proc.prog = &prog;
	user_proc.m.size = prog.m->size;
	user_proc.m.start = user_proc.pi = prog.pi;

	/* initialize memory pool for threads stacks */
	user_proc.stack_pool = ffs_init (
		U2K_GET_ADR ( user_proc.pi->stack, &user_proc ),
		(size_t) prog.pi->end_adr - (size_t) prog.pi->stack + 1 );

	prio = prog.pi->prio;
	if ( !prio )
		prio = THREAD_DEF_PRIO;

	kthread_create ( user_proc.pi->init, NULL, 0, SCHED_FIFO, prio, NULL,
			 NULL, 0, &user_proc );

	kthreads_schedule ();
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
kthread_t *kthread_create ( void *start_routine, void *arg, uint flags,
	int sched_policy, int sched_priority, sched_supp_t *sched_param,
	void *stackaddr, size_t stacksize, kprocess_t *proc )
{
	ASSERT ( proc );

	kthread_t *kthread;

	/* thread descriptor */
	kthread = kmalloc ( sizeof (kthread_t) );
	ASSERT ( kthread );

	/* initialize thread descriptor */
	kthread->id = k_new_id ();

	kthread->proc = proc;
	kthread->proc->thread_count++;

	kthread->queue = NULL;
	kthreadq_init ( &kthread->join_queue );

	kthread_create_new_state ( kthread, start_routine, arg,
				   stackaddr, stacksize, FALSE );
	kthread->state.flags = flags;

	list_init ( &kthread->states );

	/* connect signal mask in descriptor with state */
	kthread->sig_handling.mask = &kthread->state.sigmask;
	ksignal_thread_init ( kthread );

	list_append ( &all_threads, kthread, &kthread->all );

	kthread->sched_policy = sched_policy;
	if ( sched_priority < 0 )
		sched_priority = 0;
	if ( sched_priority >= PRIO_LEVELS )
		sched_priority = PRIO_LEVELS - 1;
	kthread->sched_priority = sched_priority;

	kthread->ref_cnt = 1;
	kthread_move_to_ready ( kthread, LAST );

	ksched2_thread_add (kthread, sched_policy, sched_priority, sched_param);

	return kthread;
}

/*! create/insert state for signal handler and similar */
void kthread_create_new_state ( kthread_t *kthread,
				void *start_func, void *param,
				void *stack, size_t stack_size,
				int save_old_state )
{
	ASSERT ( kthread );
	kprocess_t *proc = kthread_get_process ( kthread );
	ASSERT ( proc );

	/* save old state if requested (put it at beginning of state list) */
	if ( save_old_state )
	{
		kthread_state_t *state = kmalloc ( sizeof (kthread_state_t) );
		*state = kthread->state;
		list_prepend ( &kthread->states, state, &state->list );
	}

	int stack_provided = FALSE;

	if ( stack && stack_size )
	{
		stack_provided = TRUE;
	}
	else if ( proc->stack_pool )
	{
		/* use process stack heap */
		if ( !stack_size )
			stack_size = DEFAULT_THREAD_STACK_SIZE;
		stack = ffs_alloc ( proc->stack_pool, stack_size );
	}
	else {
		/* use kernel heap */
		if ( !stack_size )
			stack_size = DEFAULT_THREAD_STACK_SIZE;

		stack = kmalloc ( stack_size );
	}
	ASSERT ( stack && stack_size );

	if ( stack_provided )
	{
		kthread->state.stack = NULL;
		kthread->state.stack_size = 0;
	}
	else {
		kthread->state.stack = stack;
		kthread->state.stack_size = stack_size;
	}

	/* reserve space for errno in user space */
	stack_size -= sizeof (int);
	kthread->state.errno = kthread->state.stack + stack_size;

	arch_create_thread_context ( &kthread->state.context, start_func, param,
				     proc->pi->exit, stack, stack_size, proc );

	*kthread->state.errno = 0;
	kthread->state.exit_status = NULL;
	kthread->state.pparam = NULL;

	list_init ( &kthread->state.cleanup );
}

/*! restore previously saved state (last saved) */
int kthread_restore_state ( kthread_t *kthread )
{
	ASSERT ( kthread );

	kthread_state_t *state;
	/* perform cleanup of current state */
	kthread_state_cleanup_t *iter;
	while ( (iter = list_remove ( &kthread->state.cleanup, FIRST, NULL )) )
	{
		iter->cleanup ( iter->param1, iter->param2, iter->param3 );
		kfree (iter);
	}

	/* release thread stack */
	if ( kthread->state.stack )
	{
		if ( kthread->proc->stack_pool ) /* user level thread */
			ffs_free ( kthread->proc->stack_pool,
				   kthread->state.stack );
		else /* kernel level thread */
			kfree ( kthread->state.stack );
	}

	int retval = FALSE;

	/* overwrite state with first from list (if not empty) */
	state = list_remove ( &kthread->states, FIRST, NULL );
	if ( state )
	{
		kthread->state = *state;
		kfree ( state );
		retval = TRUE;
	}

	return retval;
}

/*! Suspend thread (kthreads_schedule must follow this call) */
inline int kthread_suspend (kthread_t *kthread,void *wakeup_action,void *param)
{
	if ( !kthread )
		kthread = active_thread;

	ASSERT ( kthread->state.state == THR_STATE_ACTIVE ||
		 kthread->state.state == THR_STATE_READY );

	if ( kthread->state.state == THR_STATE_READY )
		kthread_remove_from_ready ( kthread );

	kthread->state.state = THR_STATE_SUSPENDED;
	kthread->state.cancel_suspend_handler = wakeup_action;
	kthread->state.cancel_suspend_param = param;

	return 0;
}

/*! add cleanup functions for when thread terminates or finish special processing
 *  like signal handlers */
void kthread_add_cleanup ( kthread_t *kthread, void *cleanup_function,
			   param_t param1, param_t param2, param_t param3 )
{
	ASSERT ( kthread );

	kthread_state_cleanup_t *cleanup;
	cleanup = kmalloc ( sizeof (kthread_state_cleanup_t) );

	cleanup->cleanup = cleanup_function;
	cleanup->param1 = param1;
	cleanup->param2 = param2;
	cleanup->param3 = param3;

	list_append ( &kthread->state.cleanup, cleanup, &cleanup->list );
}

/*! kfree for cleanup */
void kthread_param_free ( param_t p1, param_t p2, param_t p3 )
{
	ffs_free ( p1.p_ptr, p2.p_ptr );
}

/*!
 * Cancel thread (or restore it to previous state)
 * \param kthread Thread descriptor
 */
int kthread_exit ( kthread_t *kthread, void *exit_status, int force )
{
	kthread_state_t *prev;
	int restored = FALSE;
	kthread_t *released;
	kthread_q *q;
	void **p;

	ASSERT ( kthread );
	do {
		prev = list_get ( &kthread->states, FIRST );
		if ( prev )
			restored = kthread_restore_state ( kthread );
	}
	while ( prev && force ); /* if cancel is called, destroy all states */

	if ( restored && !force ) /* restored to previous state */
	{
		if ( kthread == active_thread )
			kthread->state.state = THR_STATE_ACTIVE;

		kthreads_schedule ();

		return EXIT_SUCCESS;
	}

	if ( kthread->state.state == THR_STATE_PASSIVE )
		return EXIT_SUCCESS; /* thread is already finished */

	if ( kthread->state.state == THR_STATE_READY )
	{
		/* remove target 'thread' from its queue */
		if ( !kthread_remove_from_ready ( kthread ) )
			ASSERT ( FALSE );
	}
	else if ( kthread->state.state == THR_STATE_WAIT )
	{
		/* remove target 'thread' from its queue */
		if ( !kthreadq_remove ( kthread->queue, kthread ) )
			ASSERT ( FALSE );
	}
	else if ( kthread->state.state == THR_STATE_SUSPENDED )
	{
		/* cancellation routine */
		if ( kthread->state.cancel_suspend_handler )
			kthread->state.cancel_suspend_handler ( kthread,
				kthread->state.cancel_suspend_param );
	}
	else if ( kthread->state.state == THR_STATE_ACTIVE )
	{
		if ( kthread != active_thread )
			return ESRCH; /* thread descriptor corrupted ! */
	}
	else {
		return ESRCH; /* thread descriptor corrupted ! */
	}

	kthread->state.state = THR_STATE_PASSIVE;
	kthread->ref_cnt--;
	kthread->state.exit_status = exit_status;
	kthread->proc->thread_count--;

	/* remove it from its scheduler */
	ksched2_thread_remove ( kthread );

	arch_destroy_thread_context ( &kthread->state.context );

	kthread_restore_state ( kthread );

	if ( !kthread->ref_cnt )
	{
		kthread_remove_descriptor ( kthread );
	}
	else {
		q = &kthread->join_queue;

		while ( (released = kthreadq_remove ( q, NULL )) != NULL )
		{
			/* save exit status to all waiting threads */
			p = kthread_get_private_param ( released );
			if ( p )
				*p = exit_status;

			kthread_move_to_ready ( released, LAST );
		}
	}
	if ( kthread == active_thread )
	{
		active_thread = NULL;
		kthreads_schedule ();
	}

	return EXIT_SUCCESS;
}

/*! Internal function for removing (freeing) thread descriptor */
static void kthread_remove_descriptor ( kthread_t *kthread )
{
	ASSERT ( kthread );

	k_free_id ( kthread->id );
	kthread->id = 0;

#ifdef DEBUG
	ASSERT( kthread == list_find_and_remove (&all_threads, &kthread->all) );
#else
	(void) list_remove ( &all_threads, 0, &kthread->all );
#endif

	kfree ( kthread );
}

/*!
 * Put given thread or active thread (when kthread == NULL) into queue of
 * "waited" thread
 */
void kthread_wait_thread ( kthread_t *waiting, kthread_t *waited )
{
	ASSERT ( waited );

	waited->ref_cnt++;
	kthread_enqueue ( waiting, &waited->join_queue );
}

/*! Get exit status of finished thread (and free descriptor) */
void kthread_collect_status ( kthread_t *waited, void **retval )
{
	ASSERT ( waited );

	if ( retval )
		*retval = waited->state.exit_status;

	waited->ref_cnt--;
	if ( !waited->ref_cnt )
		kthread_remove_descriptor ( waited );
}



/*! operations on thread queues (blocked threads) --------------------------- */

/*!
 * Put given thread or active thread (when kthread == NULL) into queue 'q_id'
 * - if kthread != NULL, thread must not be in any list and 'kthreads_schedule'
 *   should follow this call before exiting from kernel!
 */
void kthread_enqueue ( kthread_t *kthread, kthread_q *q )
{
	ASSERT ( ( kthread || active_thread ) && q );

	if ( !kthread )
		kthread = active_thread;

	kthread->state.state = THR_STATE_WAIT;
	kthread->queue = q;

	kthreadq_append ( kthread->queue, kthread );
}

/*!
 * Release single thread from given queue (if queue not empty)
 * \param q Queue
 * \return 1 if thread was released, 0 if queue was empty
 */
int kthreadq_release ( kthread_q *q )
{
	kthread_t *kthread;

	ASSERT ( q );

	kthread = kthreadq_remove ( q, NULL );

	if ( kthread )
	{
		kthread_move_to_ready ( kthread, LAST );
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
int kthreadq_release_all ( kthread_q *q )
{
	ASSERT ( q );
	int cnt = 0;

	while ( kthreadq_release (q) )
		cnt++;

	return cnt;
}

/*! thread queue manipulation */
inline void kthreadq_init ( kthread_q *q )
{
	ASSERT ( q );
	list_init ( &q->q );
}
inline void kthreadq_append ( kthread_q *q, kthread_t *kthread )
{
	ASSERT ( kthread && q );
	list_append ( &q->q, kthread, &kthread->list );
}
inline void kthreadq_prepend ( kthread_q *q, kthread_t *kthread )
{
	ASSERT ( kthread && q );
	list_prepend ( &q->q, kthread, &kthread->list );
}
inline kthread_t *kthreadq_remove ( kthread_q *q, kthread_t *kthread )
{
	ASSERT ( q );
	if ( kthread )
		return list_find_and_remove ( &q->q, &kthread->list );
	else
		return list_remove ( &q->q, FIRST, NULL );
}
inline kthread_t *kthreadq_get ( kthread_q *q )
{
	ASSERT ( q );
	return list_get ( &q->q, FIRST );
}
inline kthread_t *kthreadq_get_next ( kthread_t *kthread )
{
	ASSERT ( kthread );
	return list_get_next ( &kthread->list );   /* kthread->queue->q.first->object */
}

/*! get thread scheduling policy */
inline int kthread_get_sched_policy ( kthread_t *kthread )
{
	if ( kthread )
		return kthread->sched_policy;
	else
		return active_thread->sched_policy;
}

inline int kthread_get_prio ( kthread_t *kthread )
{
	if ( kthread )
		return kthread->sched_priority;
	else
		return active_thread->sched_priority;
}
int kthread_set_prio ( kthread_t *kthread, int prio )
{
	kthread_t *kthr = kthread;
	int old_prio;

	if ( !kthr )
		kthr = active_thread;

	old_prio = kthr->sched_priority;

	/* change thread priority:
	(i)	if its active: change priority and move to ready
	(ii)	if its ready: remove from queue, change priority, put back
	(iii)	if its blocked: if queue is sorted by priority, same as (ii)
	*/
	switch ( kthr->state.state )
	{
	case THR_STATE_ACTIVE:
		kthr->sched_priority = prio;
		kthread_move_to_ready ( kthr, LAST );
		kthreads_schedule ();
		break;

	case THR_STATE_READY:
		kthread_remove_from_ready (kthr);
		kthr->sched_priority = prio;
		kthread_move_to_ready ( kthr, LAST );
		kthreads_schedule ();
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

inline int kthread_is_active ( kthread_t *kthread )
{
	ASSERT ( kthread );
	if ( kthread->state.state == THR_STATE_ACTIVE )
		return TRUE;
	else
		return FALSE;
}

inline int kthread_is_ready ( kthread_t *kthread )
{
	ASSERT ( kthread );

	return	kthread->state.state == THR_STATE_ACTIVE ||
		kthread->state.state == THR_STATE_READY;
}

inline int kthread_is_alive ( kthread_t *kthread )
{
	return	kthread->state.state != THR_STATE_PASSIVE &&
		kthread_check_kthread ( kthread );
}

inline int kthread_is_suspended (kthread_t *kthread, void **func, void **param)
{
	if ( !kthread )
		kthread = active_thread;

	if ( kthread->state.state == THR_STATE_SUSPENDED )
	{
		if ( func )
			*func = kthread->state.cancel_suspend_handler;
		if ( param )
			*param = kthread->state.cancel_suspend_param;

		return TRUE;
	}

	return FALSE;
}

/*! check if thread descriptor is valid, i.e. is in all thread list */
inline int kthread_check_kthread ( kthread_t *kthread )
{
	return kthread && list_find ( &all_threads, &kthread->all );
}

inline int kthread_get_id ( kthread_t *kthread )
{
	if ( kthread )
		return kthread->id;
	else
		return active_thread->id;
}

inline kthread_t *kthread_get_active ()
{
	return active_thread;
}

inline void *kthread_get_context ( kthread_t *kthread )
{
	if ( kthread )
		return &kthread->state.context;
	else
		return &active_thread->state.context;
}

inline void *kthread_get_process ( kthread_t *kthread )
{
	if ( kthread )
		return kthread->proc;
	else
		return active_thread->proc;
}

/*! Get kernel thread descriptor from user thread descriptor */
inline kthread_t *kthread_get_descriptor ( pthread_t *thread )
{
	kthread_t *kthread;

	if ( thread && (kthread = thread->ptr) && thread->id == kthread->id &&
		kthread->state.state != THR_STATE_PASSIVE )
		return kthread;
	else
		return NULL;
}

inline void *kthread_get_sched2_param ( kthread_t *kthread )
{
	if ( kthread )
		return &kthread->sched2;
	else
		return &active_thread->sched2;
}

inline void *kthread_get_sigparams ( kthread_t *kthread )
{
	if ( !kthread )
		kthread = active_thread;
	ASSERT ( kthread );
	return &kthread->sig_handling;
}

inline void kthread_set_active ( kthread_t *kthread )
{
	ASSERT ( kthread );
	active_thread = kthread;
	active_thread->state.state = THR_STATE_ACTIVE;
	active_thread->queue = NULL;
}
inline void kthread_mark_ready ( kthread_t *kthread )
{
	ASSERT ( kthread );
	kthread->state.state = THR_STATE_READY;
}
inline void kthread_set_queue ( kthread_t *kthread, kthread_q *queue )
{
	ASSERT ( kthread && queue );
	kthread->queue = queue;
}
inline kthread_q *kthread_get_queue ( kthread_t *kthread )
{
	ASSERT ( kthread );
	return kthread->queue;
}


/*! Temporary storage for blocked thread (save specific context before wait) */
inline void kthread_set_private_param ( kthread_t *kthread, void *pparam )
{
	if ( !kthread )
		kthread = active_thread;
	kthread->state.pparam = pparam;
}
inline void *kthread_get_private_param ( kthread_t *kthread )
{
	if ( !kthread )
		kthread = active_thread;
	return kthread->state.pparam;
}


/*! errno and return value */
inline void kthread_set_errno ( kthread_t *kthread, int error_number )
{
	if ( kthread )
		*kthread->state.errno = error_number;
	else
		*active_thread->state.errno = error_number;
}
inline int kthread_get_errno ( kthread_t *kthread )
{
	if ( kthread )
		return *kthread->state.errno;
	else
		return *active_thread->state.errno;
}
inline int *kthread_get_errno_ptr ( kthread_t *kthread )
{
	if ( kthread )
		return kthread->state.errno;
	else
		return active_thread->state.errno;
}
inline void kthread_set_syscall_retval ( kthread_t *kthread, int ret_val )
{
	if ( !kthread )
		kthread = active_thread;
	ASSERT ( kthread );
	arch_syscall_set_retval ( kthread_get_context(kthread), ret_val );
}


/*! display active & ready threads info on console */
int kthread_info ()
{
	kthread_t *kthread;
	int i = 1;

	kprintf ( "Threads info\n" );

	kprintf ( "[this]\tid=%d (desc. at %x) in process at %x, size=%d\n",
		  active_thread->id, active_thread, active_thread->proc->m.start,
		  active_thread->proc->m.size );

	kprintf ( "\tprio=%d, state=%d, exit_status=%x\n",
		  active_thread->sched_priority, active_thread->state.state,
		  active_thread->state.exit_status );

	kthread = list_get ( &all_threads, FIRST );
	while ( kthread )
	{
		kprintf ( "[%d]\tid=%d (desc. at %x) in process at %x, size=%d\n",
			 i++, kthread->id, kthread, kthread->proc->m.start,
			 kthread->proc->m.size );

		kprintf ( "\tprio=%d, state=%d, exit_status=%x\n",
			 kthread->sched_priority, kthread->state.state,
			 kthread->state.exit_status );

		kthread = list_get_next ( &kthread->all );
	}

	return 0;
}

/*! Idle thread ------------------------------------------------------------- */
#include <api/syscall.h>

/*! Idle thread starting (and only) function */
static void idle_thread ( void *param )
{
	while (1)
		syscall ( SUSPEND, NULL );
}

/*! Change thread scheduling parameters ------------------------------------- */
int kthread_setschedparam (kthread_t *kthread, int policy, sched_param_t *param)
{
	int sched_priority;
	sched_supp_t *supp;

	ASSERT_ERRNO_AND_EXIT ( kthread, EINVAL );
	ASSERT_ERRNO_AND_EXIT ( kthread_is_alive (kthread), ESRCH );
	ASSERT_ERRNO_AND_EXIT ( policy >= 0 && policy < SCHED_NUM, EINVAL );

	if ( param )
	{
		ASSERT_ERRNO_AND_EXIT (
			param->sched_priority >= THREAD_MIN_PRIO &&
			param->sched_priority <= THREAD_MAX_PRIO, EINVAL );

		if ( param->sched_priority )
			sched_priority = param->sched_priority;
		else
			sched_priority = kthread->sched_priority;

		supp = &param->supp;
	}
	else {
		sched_priority = kthread->sched_priority;
		supp = NULL;
	}

	/* change in priority? */
	if ( kthread->sched_priority != sched_priority )
		kthread_set_prio ( kthread, sched_priority );

	/* change in scheduling policy? */
	if ( kthread->sched_policy != policy )
	{
		ksched2_thread_remove ( kthread );
		ksched2_schedule ( kthread->sched_policy );
		kthread->sched_policy = policy;
		ksched2_thread_add ( kthread, policy, sched_priority, supp );
		ksched2_schedule ( kthread->sched_policy );
	}
	else if ( supp ) /* if policy changed, parameters are already given */
	{
		ksched2_setsched_param ( kthread, supp );
	}

	return EXIT_SUCCESS;
}
