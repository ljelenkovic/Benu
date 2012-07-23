/*! Thread management */
#pragma once

#include <kernel/thread.h>
#include <lib/list.h>


#ifdef	_K_PTHREAD_C_

/*! monitors and conditional variables -------------------------------------- */

typedef struct _kpthread_mutex_t_
{
	id_t	    id;
		    /* system level id */

	kthread_t  *owner;
		   /* owner thread descriptor, or NULL if monitor empty */

	uint	    flags;
		    /* various flags */

	uint	    ref_cnt;
		    /* various flags */

	kthread_q   queue;
		    /* queue for blocked threads */
}
kpthread_mutex_t;

typedef struct _kpthread_cond_t_
{
	id_t	    id;
		    /* system level id */

	uint	    flags;
		    /* various flags */

	uint	    ref_cnt;
		    /* various flags */

	kthread_q   queue;
		    /* queue for blocked threads */
}
kpthread_cond_t;


/*! semaphores -------------------------------------------------------------- */

typedef struct _ksem_t_
{
	id_t	    id;
		    /* system level id */

	int	    sem_value;
		    /* current semaphore value */

	kthread_t  *last_lock;
		   /* thread that last called sem_wait and wasn't blocked */

	uint	    flags;
		    /* various flags */

	uint	    ref_cnt;
		    /* various flags */

	kthread_q   queue;
		    /* queue for blocked threads */
}
ksem_t;

#endif	/* _K_PTHREAD_C_ */
