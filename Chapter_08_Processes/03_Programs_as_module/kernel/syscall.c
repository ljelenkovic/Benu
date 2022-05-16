/*! System call - call to kernel from threads (via software interrupt) */

#define _K_SYSCALL_C_

#include "syscall.h"

#include <kernel/pthread.h>
#include <kernel/device.h>
#include <kernel/errno.h>
#include <kernel/features.h>
#include <kernel/memory.h>
#include <kernel/signal.h>
#include <kernel/time.h>

#include "thread.h"
#include <arch/syscall.h>
#include <arch/interrupt.h>
#include <arch/processor.h>

/*! syscall handlers */
static int(*k_sysfunc[SYSFUNCS])(void *params) =
{
	NULL,

	sys__sysinfo,
	sys__feature,

	sys__set_errno,
	sys__get_errno,
	sys__get_errno_ptr,

	sys__clock_gettime,
	sys__clock_settime,
	sys__clock_nanosleep,

	sys__timer_create,
	sys__timer_delete,
	sys__timer_settime,
	sys__timer_gettime,

	sys__open,
	sys__close,
	sys__read,
	sys__write,
	sys__device_status,
	sys__poll,

	sys__pthread_create,
	sys__pthread_exit,
	sys__pthread_join,
	sys__pthread_self,

	sys__pthread_setschedparam,

	sys__pthread_mutex_init,
	sys__pthread_mutex_destroy,
	sys__pthread_mutex_lock,
	sys__pthread_mutex_unlock,
	sys__pthread_cond_init,
	sys__pthread_cond_destroy,
	sys__pthread_cond_wait,
	sys__pthread_cond_signal,
	sys__pthread_cond_broadcast,

	sys__sem_init,
	sys__sem_destroy,
	sys__sem_wait,
	sys__sem_post,

	sys__mq_open,
	sys__mq_close,
	sys__mq_send,
	sys__mq_receive,

	sys__sigaction,
	sys__pthread_sigmask,
	sys__sigqueue,
	sys__sigwaitinfo
};

/*!
 * Process syscalls
 * (syscall is forwarded from arch interrupt subsystem to k_syscall)
 */
void k_syscall(uint irqn)
{
	int id, retval;
	void *context, *params;

	ASSERT(irqn == SOFTWARE_INTERRUPT);

	context = kthread_get_context(NULL); /* active thread context */

	id = arch_syscall_get_id(context);

	ASSERT(id >= 0 && id < SYSFUNCS);

	params = arch_syscall_get_params(context);

	retval = k_sysfunc[id](params);

	if (id != PTHREAD_EXIT)
		arch_syscall_set_retval(context, retval);
}

/*! Stop processor until next interrupt occurs - for idle thread only! */
int sys__suspend(void *p)
{
	enable_interrupts();
	suspend();
	disable_interrupts();

	return 0;
}
