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


/*! Messages ---------------------------------------------------------------- */

/*! message */
typedef struct _kmsg_t_
{
	list_h	list;
		/* messages are placed in list sorted by priority */
	size_t	msg_size;
		/* message size */
	int	msg_prio;
		/* message priority */
	char	msg_data[1];
		/* information saved in message (variable length)*/
}
kmq_msg_t;


/*! message queue */
typedef struct _kmq_queue_t_
{
	id_t       id;
		   /* system level id */

	char	  *name;
		   /* message queue name */

	mq_attr_t  attr;
		   /* message queue attributes */


	list_t	   msg_list;
		   /* list for messages */

	int	   ref_cnt;
		   /* number of processes that have opened this queue */

	kthread_q  recv_q;
		   /* threads waiting for messages */

	kthread_q  send_q;
		   /* threads waiting for space in queue (to store message) */

	list_h	   list;
		   /* all message queues are in single list */
}
kmq_queue_t;


#endif	/* _K_PTHREAD_C_ */
