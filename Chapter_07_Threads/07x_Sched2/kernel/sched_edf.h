/*! EDF scheduler */
#pragma once

#include "thread.h"
#include "time.h"

/*! Per thread scheduler data */
typedef struct _ksched_edf_thread_params_t
{
	timespec_t  relative_deadline;
	timespec_t  period;
	timespec_t  next_run;
	timespec_t  active_deadline;
	int         flags;

	void       *period_alarm;
		    /* kernel alarm reference used in EDF */
	void       *deadline_alarm;
}
ksched_edf_thread_params_t;

/*! EDF global parameters */
typedef struct _ksched_edf_t_
{
	kthread_t  *active; /* thread selected by EDF as top priority */

	kthread_q   ready;
	kthread_q   wait;
}
ksched_edf_t;

#define EDF_DEBUG	0	/* print extensive debug informations? */

#if EDF_DEBUG == 1
#define	EDF_LOG(...)		LOG(EDFLOG, ##__VA_ARGS__)
#else /* EDF_DEBUG */
#define	EDF_LOG(...)
#endif
