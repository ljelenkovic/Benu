/*! Schedulers parameters */
#pragma once

#include <types/basic.h>
#include <types/time.h>

/*! Additional schedulers (scheduling policies) */
enum {
	SCHED_RR = 1,
	SCHED_EDF,

	SCHED_NUM,
};

/*! Scheduler parameters used by user threads */

/*!
 * RR scheduler, actually do not need parameters to be set by user thread,
 * but is present for interface demonstration
 */
typedef struct _sched_rr_t_
{
	timespec_t  time_slice;
}
sched_rr_t;

/*!
 * EDF scheduler
 */
#define EDF_SET		(1<<0)
#define EDF_WAIT	(1<<1)
#define EDF_EXIT	(1<<2)
#define EDF_TERMINATE	(1<<3)
#define EDF_CONTINUE	(1<<4)
#define EDF_SKIP	(1<<5)

typedef struct _sched_edf_t_
{
	timespec_t  deadline;
	timespec_t  period;
	int         flags;
}
sched_edf_t;

/*!
 * Supplement scheduling parameters definable by thread
 * (beside policy and priority)
 */
typedef union _sched_t_
{
	sched_rr_t   rr;
	sched_edf_t  edf;
}
sched_supp_t;
