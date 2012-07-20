/*! Threads */
#pragma once

#include <types/basic.h>

#include <types/sched2.h>

/*! POSIX thread descriptor (user space) */
typedef descriptor_t pthread_t;
typedef pthread_t pid_t;

/*! Scheduling parameters */
typedef struct sched_param
{
	int           sched_priority;
		      /* thread priority */

	sched_supp_t  supp;
		      /* additional scheduling parameters for policy != FIFO */
}
sched_param_t;

#define SCHED_FIFO		0

#define THREAD_MIN_PRIO		0
#define THREAD_MAX_PRIO		(PRIO_LEVELS - 1)
#define THREAD_DEF_PRIO		THR_DEFAULT_PRIO

/*! Attributes for pthread_attr_t, parameter for pthread_create */
typedef struct pthread_attr
{
	uint	       flags;
	int	       sched_policy;
	sched_param_t  sched_params;

	void	      *stackaddr;
	size_t	       stacksize;
}
pthread_attr_t;

/* flags for pthread_attr_t */
#define	PTHREAD_CREATE_DETACHED		(1<<0)
#define	PTHREAD_CREATE_JOINABLE		(1<<1)
#define	PTHREAD_EXPLICIT_SCHED		(1<<2)
#define	PTHREAD_INHERIT_SCHED		(1<<3)
#define	PTHREAD_SCOPE_SYSTEM		(1<<4)
#define	PTHREAD_SCOPE_PROCESS		(1<<5)

/*! Mutex */
typedef descriptor_t pthread_mutex_t;

/*! Mutex creation parameters */
typedef uint pthread_mutexattr_t;
#define	PTHREAD_PROCESS_SHARED		(1<<6)
#define	PTHREAD_PROCESS_PRIVATE		(1<<7)

/*! Condition variable */
typedef descriptor_t pthread_cond_t;

/*! Condition variable creation parameters */
typedef uint pthread_condattr_t;

/*! Semaphore */
typedef descriptor_t sem_t;

/*! Message queue */
typedef descriptor_t mqd_t;

/*! Message queue parameters */
typedef struct mq_attr
{
	int  mq_flags;
	     /* Message queue flags */
	int  mq_maxmsg;
	     /* Maximum number of messages */
	int  mq_msgsize;
	     /* Maximum message size */
	int  mq_curmsgs;
	     /* Number of messages currently queued */
}
mq_attr_t;

#define MQ_PRIO_MAX	255
