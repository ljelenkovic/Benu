/*! Signals */
#pragma once

#include <kernel/signal.h>
#include <lib/list.h>

struct _ksignal_handling_t_;
typedef struct _ksignal_handling_t_ ksignal_handling_t;

#include "thread.h"

/*! interface to kernel */
int ksignal_thread_init(kthread_t *kthread);
int ksignal_queue(kthread_t *receiver, siginfo_t *sig);
int ksignal_process_pending(kthread_t *kthread);
int ksignal_process_event(sigevent_t *evp, kthread_t *kthread, int code);

struct _ksignal_handling_t_
{
	sigset_t    *mask;
		     /* addres of mask saved in thread state */
	sigaction_t  act[SIGMAX];

	list_t	     pending_signals;
		     /* siginfo_t elements */
};

#ifdef	_K_SIGNAL_C_
/*! rest of the file is only for 'kernel/signal.c' -------------------------- */

/* for queuing signals to thread */
typedef struct _ksiginfo_t_
{
	siginfo_t  siginfo;
	list_h	   list;
}
ksiginfo_t;

#endif	/* _K_SIGNAL_C_ */
