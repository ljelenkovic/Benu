/*! Signals */
#define _K_SIGNAL_C_

#include "signal.h"

#include "thread.h"
#include <arch/context.h>
#include <kernel/kprint.h>
#include <kernel/errno.h>
#include "time.h"

static int ksignal_received_signal(kthread_t *kthread, void *param);
static void ksignal_add_to_pending(ksignal_handling_t *sh, siginfo_t *sig);

/*! Initialize thread signal handling data */
int ksignal_thread_init(kthread_t *kthread)
{
	ksignal_handling_t *sh;

	ASSERT(kthread);

	sh = kthread_get_sigparams(kthread);

	sigfillset(sh->mask); /* all signals are blocked */

	list_init(&sh->pending_signals);

	return EXIT_SUCCESS;
}

/*! Send signal to target thread */
int ksignal_queue(kthread_t *kthread, siginfo_t *sig)
{
	int enqueue = FALSE;
	int retval = EXIT_SUCCESS;
	int schedule = FALSE;
	ksignal_handling_t *sh;
	sigaction_t *act;
	void (*func)(kthread_t *, void *), *param;
	siginfo_t *us;
	param_t param1, param2, param3;
	context_t context;

	ASSERT(kthread);
	ASSERT(kthread_check_kthread(kthread));
	ASSERT(sig->si_signo > 0 && sig->si_signo <= SIGMAX);

	if (!kthread_is_alive(kthread))
		return ESRCH;

	sh = kthread_get_sigparams(kthread);

	/* is thread suspended and waits for this signal? */
	if (
		kthread_is_suspended(kthread, (void **) &func, &param) &&
		((void *) func) == ((void *) ksignal_received_signal)
	)
	{
		/* thread is waiting for signal */
		if (!ksignal_received_signal(kthread, sig))
		{
			/* waited for this signal */

			/* do not process this signal further */
			return EXIT_SUCCESS;
		}
	}

	/*
	 * If thread is in interruptable state and signal is not masked:
	 * - process signal
	 * - otherwise queue it
	 */
	if (
		kthread_get_interruptable(kthread) &&
		!sigtestset(sh->mask, sig->si_signo)
	)
	{
		act = &sh->act[sig->si_signo];

		if (act->sa_flags != SA_SIGINFO)
			return ENOTSUP; /* not supported without SA_SIGINFO! */

		if (	act->sa_sigaction == SIG_ERR ||
			act->sa_sigaction == SIG_DFL ||
			act->sa_sigaction == SIG_IGN ||
			act->sa_sigaction == SIG_HOLD	)
		{
			return ENOTSUP; /* not yet supported */
		}

		if (!kthread_is_ready(kthread))
		{
			/*
			 * thread is suspended/blocked on something else
			 *
			 * 1. handle interruption:
			 *    a) we break suspend/wait state
			 *      (call cancel function)
			 *    b) set errno = EINTR
			 *    c) set return value = FAILURE
			 * 2. create new thread state
			 *    a) process signal in this state
			 */
			void (*func)(kthread_t *, void *), *param;
			kthread_is_suspended(kthread, (void **) &func, &param);

			if (func)
				func(kthread, param);

			kthread_move_to_ready(kthread, LAST);
			kthread_set_errno(kthread, EINTR);
			kthread_set_syscall_retval(kthread, EXIT_FAILURE);

			/* thread is unsuspended, but signal
			 * handler will be added first */
			schedule = TRUE;
		}

		/* copy sig */
		us = kmalloc(sizeof(siginfo_t));
		ASSERT(us); /* if (!us) return ENOMEM; */

		*us = *sig;

		/* if sender is recipient, remember context */
		if (kthread_is_active(kthread))
			context = *((context_t *) kthread_get_context(kthread));

		kthread_create_new_state(kthread, act->sa_sigaction, us, NULL,
					   HANDLER_STACK_SIZE, TRUE);

		param1.p_ptr = us;
		param2.p_ptr = NULL;
		param3.p_ptr = NULL;
		kthread_add_cleanup(kthread, kthread_param_free,
				      param1, param2, param3);

		/* mask signal in thread mask */
		sigaddset(sh->mask, sig->si_signo);
		/* mask additional signals in thread mask */
		sigaddsets(sh->mask, &act->sa_mask);

		/* if active thread is recipient switch to new state */
		if (kthread_is_active(kthread))
		{
			arch_switch_to_thread(&context,
						kthread_get_context(kthread));
		}
	}
	else {
		enqueue = TRUE;
	}

	if (enqueue)
	{
		ksignal_add_to_pending(sh, sig);

		retval = EAGAIN;
	}

	if (schedule)
		kthreads_schedule();

	return retval;
}

static void ksignal_add_to_pending(ksignal_handling_t *sh, siginfo_t *sig)
{
	ksiginfo_t *ksig;

	/* add signal to list of pending signals */
	ksig = kmalloc(sizeof(ksiginfo_t));
	ksig->siginfo = *sig;

	list_append(&sh->pending_signals, ksig, &ksig->list);
	/* list_sort_add(&sh->pending_signals, ksig, &ksig->list,
			ksignal_compare); */
}

/*! Process pending signals for thread (called from kthreads_schedule()) */
int ksignal_process_pending(kthread_t *kthread)
{
	ksignal_handling_t *sh;
	int retval = EXIT_SUCCESS;
	ksiginfo_t *ksig, *next;

	ASSERT(kthread);

	sh = kthread_get_sigparams(kthread);

	ksig = list_get(&sh->pending_signals, FIRST);
	while (ksig)
	{
		next = list_get_next(&ksig->list);

		if (!sigtestset(sh->mask, ksig->siginfo.si_signo))
		{
			list_remove(&sh->pending_signals, 0, &ksig->list);

			retval = ksignal_queue(kthread, &ksig->siginfo);

			kfree(ksig);

			/* handle only first signal?
				* no, all of them - they will mask ... */
			/*if (retval == EXIT_SUCCESS)
				break;*/
		}

		ksig = next;
	}

	return retval;
}

/*! Callback function called when a signal is delivered to suspended thread */
static int ksignal_received_signal(kthread_t *kthread, void *param)
{
	siginfo_t *sig;
	ksignal_handling_t *sh;

	ASSERT(kthread);

	/* thread waked by signal or other event? */
	if (param == NULL)
	{
		kthread_set_errno(kthread, EINTR);
		kthread_set_syscall_retval(kthread, EXIT_FAILURE);

		return EXIT_FAILURE; /* other event interrupted thread */
	}

	/* signal interrupted, but did thread waited for this signal? */
	sig = param;
	sh = kthread_get_sigparams(kthread);

	/* thread is suspended and waiting for signal
	 * (FIXME if other reasons can lead to this point) */

	/* add signal to pending and resume suspended thread
	 * (which will then handle signal) */

	ksignal_add_to_pending(sh, sig);

	/* resume thread */
	kthread_set_errno(kthread, EAGAIN);
	kthread_set_syscall_retval(kthread, EXIT_FAILURE);
	kthread_move_to_ready(kthread, LAST);

	kthreads_schedule();

	return EXIT_SUCCESS;
}

/*! Process event defined with sigevent_t */
int ksignal_process_event(sigevent_t *evp, kthread_t *kthread, int code)
{
	int retval = EXIT_SUCCESS;
	kthread_t *target = kthread;
	siginfo_t sig;
	pid_t pid;
	void (*func)(sigval_t);

	ASSERT(evp && kthread);

	switch(evp->sigev_notify)
	{
	case SIGEV_WAKE_THREAD:
		func = evp->sigev_notify_function;
		func(evp->sigev_value);
		break;

	case SIGEV_NONE:
		break;

	case SIGEV_THREAD_ID:
		pid = evp->sigev_notify_thread_id;
		target = pid.ptr;

		if (!target || !kthread_is_alive(target) ||
			pid.id != kthread_get_id(target))
			return ESRCH;

	case SIGEV_SIGNAL:
		sig.si_signo = evp->sigev_signo;
		sig.si_value = evp->sigev_value;
		sig.si_code = code;
		sig.si_errno = 0;
		sig.si_pid.id = kthread_get_id(kthread);
		sig.si_pid.ptr = kthread;

		retval = ksignal_queue(target, &sig);

		break;

	case SIGEV_THREAD:
		if (evp->sigev_notify_function)
		{
			if (!kthread_create(	evp->sigev_notify_function,
						evp->sigev_value.sival_ptr, 0,
						SCHED_FIFO, THREAD_DEF_PRIO,
						NULL, 0))
				retval = EINVAL;
		}
		else {
			retval = EINVAL;
		}
		break;

	default:
		retval = EINVAL;
		break;
	}

	return retval;
}


/*! Interface to threads ---------------------------------------------------- */

/*!
 * Send signal to thread (from thread)
 * \param pid Thread descriptor (user level descriptor)
 * \param signo Signal number
 * \param sigval Parameter to send with signal
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sigqueue(pid_t pid, int signo, sigval_t sigval)
{
	pthread_t thread, sender;
	kthread_t *kthread;
	siginfo_t sig;
	int retval;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(signo > 0 && signo <= SIGMAX, EINVAL);

	thread = (pthread_t) pid; /* pid_t should be pthread_t */
	ASSERT_ERRNO_AND_EXIT(thread.ptr, EINVAL);

	kthread = thread.ptr;
	ASSERT_ERRNO_AND_EXIT(kthread_get_id(kthread) == thread.id, EINVAL);
	ASSERT_ERRNO_AND_EXIT(kthread_check_kthread(kthread), EINVAL);

	sender.id = kthread_get_id(NULL);
	sender.ptr = kthread_get_active();

	sig.si_signo = signo;
	sig.si_value = sigval;
	sig.si_pid = sender;
	sig.si_code = SI_USER;
	sig.si_errno = 0;

	retval = ksignal_queue(kthread, &sig);

	if (retval == EXIT_SUCCESS)
		SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
	else
		SYS_EXIT(retval, EXIT_FAILURE);
}

/*!
 * Modify thread signal mask
 * \param how How to modify current must
 * \param set Mask to use in modification
 * \param oset Where to store old mask
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__pthread_sigmask(int how, sigset_t *set, sigset_t *oset)
{
	ksignal_handling_t *sh;
	int retval = EXIT_SUCCESS;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(set, EINVAL);

	sh = kthread_get_sigparams(NULL);

	if (oset)
		*oset = *sh->mask;

	switch(how)
	{
	case SIG_BLOCK:
		sigaddsets(sh->mask, set);
		break;

	case SIG_UNBLOCK:
		sigaddsets(sh->mask, set);
		break;

	case SIG_SETMASK:
		*sh->mask = *set;
		break;

	default:
		retval = EINVAL;
	}

	/* reevaluate pending signals with new mask */
	if (retval == EXIT_SUCCESS)
		ksignal_process_pending(kthread_get_active());

	if (retval == EXIT_SUCCESS)
		SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
	else
		SYS_EXIT(retval, EXIT_FAILURE);
}

/*!
 * Modify thread signal mask
 * \param sig Signal number
 * \param act Signal handling behavior
 * \param oact Where to save old signal behavior
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sys__sigaction(int sig, sigaction_t *act, sigaction_t *oact)
{
	ksignal_handling_t *sh;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(sig > 0 && sig <= SIGMAX, EINVAL);

	sh = kthread_get_sigparams(NULL);

	if (oact)
		*oact = sh->act[sig];

	if (act)
	{
		if (!(act->sa_flags & SA_SIGINFO))
			SYS_EXIT(ENOTSUP, EXIT_FAILURE);

		if (	act->sa_sigaction == SIG_ERR ||
			act->sa_sigaction == SIG_DFL ||
			act->sa_sigaction == SIG_IGN ||
			act->sa_sigaction == SIG_HOLD	)
		{
			SYS_EXIT(ENOTSUP, EXIT_FAILURE);
		}

		sh->act[sig] = *act;
		sigdelset(sh->mask, sig); /* accept sig */

		/* reevaluate pending signals with changed behavior */
		ksignal_process_pending(kthread_get_active());
	}

	SYS_EXIT(EXIT_SUCCESS, EXIT_SUCCESS);
}

/*!
 * Wait for signal
 * \param set Signals thread is waiting for
 * \param info Where to save caught signal
 * \return signal number if signal is caught,
 *         -1 otherwise and appropriate error number is set
 */
int sys__sigwaitinfo(sigset_t *set, siginfo_t *info)
{
	kthread_t *kthread;
	ksignal_handling_t *sh;
	ksiginfo_t *ksig, *next;
	int retval;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(set, EINVAL);

	kthread = kthread_get_active();
	sh = kthread_get_sigparams(kthread);

	do {
		/* first, search for such signal in pending signals */
		ksig = list_get(&sh->pending_signals, FIRST);
		while (ksig)
		{
			next = list_get_next(&ksig->list);

			if (sigtestset(set, ksig->siginfo.si_signo))
			{
				retval = ksig->siginfo.si_signo;
				if (info)
					*info = ksig->siginfo;

				list_remove(&sh->pending_signals,
					      0, &ksig->list);
				kfree(ksig);

				SYS_EXIT(EXIT_SUCCESS, retval);
			}

			ksig = next;
		}

		/*
		* if no pending signal found that matches given mask
		* suspend thread until signal is received
		*/
		kthread_suspend(kthread, ksignal_received_signal, NULL);

		kthread_set_errno(kthread, EINTR);
		kthread_set_syscall_retval(kthread, EXIT_FAILURE);

		kthreads_schedule();
	}
	while (kthread_get_errno(NULL) == EAGAIN);

	SYS_EXIT(kthread_get_errno(NULL), kthread_get_syscall_retval(NULL));
}

