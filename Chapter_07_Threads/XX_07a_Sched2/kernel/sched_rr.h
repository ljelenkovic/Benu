/*! Round Robin scheduler */
#pragma once

#include "time.h"

/*! Per thread scheduler data */
typedef struct _ksched_rr_thread_params_
{
	timespec_t  slice_start;
	timespec_t  slice_end;
	timespec_t  remainder;
}
ksched_rr_thread_params;

/*! Round Robin global parameters */
typedef struct _ksched_rr_t_
{
	timespec_t    time_slice;
		      /* time slice each thread is given at start */

	timespec_t    threshold;
		      /* if remaining time is less than threshold do not return
		       * to that thread, but schedule next one */

	void	     *rr_alarm;
		      /* kernel alarm reference used in RR */

	ktimer_t     *ktimer;
		      /* timer */

	itimerspec_t  alarm;
		      /* alarm parameters */
}
ksched_rr_t;
