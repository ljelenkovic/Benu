/*! Secondary scheduler interface
 *
 * Native scheduler implemented in kernel is priority scheduler with FIFO for
 * threads with same priority. Secondary schedulers can influence scheduling by
 * adjusting priority of tasks (threads) they are "scheduling".
 * For more information on how to implement particular scheduler look at example
 * given with Round Robin scheduling (sched_rr.h/c).
 */
#pragma once

#include <types/pthread.h>

/*! Interface to threads ---------------------------------------------------- */

int sys__set_sched_params ( void *p );
int sys__get_sched_params ( void *p );
int sys__set_thread_sched_params ( void *p );
int sys__get_thread_sched_params ( void *p );
