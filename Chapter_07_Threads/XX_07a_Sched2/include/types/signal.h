/*! Signals - "simplified" version of linux signal.h (with includes) */
#pragma once

#include <types/basic.h>
#include <types/pthread.h>

/* Signals for POSIX "compatibility" (not used as defined!) */
#define SIGHUP		1	/* Hangup (POSIX) */
#define SIGINT		2	/* Interrupt (ANSI) */
#define SIGQUIT		3	/* Quit (POSIX) */
#define SIGILL		4	/* Illegal instruction (ANSI) */
#define SIGTRAP		5	/* Trace trap (POSIX) */
#define SIGABRT		6	/* Abort (ANSI) */
#define SIGIOT		6	/* IOT trap (4.2 BSD) */
#define SIGBUS		7	/* BUS error (4.2 BSD) */
#define SIGFPE		8	/* Floating-point exception (ANSI) */
#define SIGKILL		9	/* Kill, unblockable (POSIX) */
#define SIGUSR1		10	/* User-defined signal 1 (POSIX) */
#define SIGSEGV		11	/* Segmentation violation (ANSI) */
#define SIGUSR2		12	/* User-defined signal 2 (POSIX) */
#define SIGPIPE		13	/* Broken pipe (POSIX) */
#define SIGALRM		14	/* Alarm clock (POSIX) */
#define SIGTERM		15	/* Termination (ANSI) */
#define SIGSTKFLT	16	/* Stack fault */
#define SIGCLD		SIGCHLD	/* Same as SIGCHLD (System V) */
#define SIGCHLD		17	/* Child status has changed (POSIX) */
#define SIGCONT		18	/* Continue (POSIX) */
#define SIGSTOP		19	/* Stop, unblockable (POSIX) */
#define SIGTSTP		20	/* Keyboard sstop (POSIX) */
#define SIGTTIN		21	/* Background read from tty (POSIX) */
#define SIGTTOU		22	/* Background write to tty (POSIX) */
#define SIGURG		23	/* Urgent condition on socket (4.2 BSD) */
#define SIGXCPU		24	/* CPU limit exceeded (4.2 BSD) */
#define SIGXFSZ		25	/* File size limit exceeded (4.2 BSD) */
#define SIGVTALRM	26	/* Virtual alarm clock (4.2 BSD) */
#define SIGPROF		27	/* Profiling alarm clock (4.2 BSD) */
#define SIGWINCH	28	/* Window size change (4.3 BSD, Sun) */
#define SIGPOLL		SIGIO	/* Pollable event occurred (System V) */
#define SIGIO		29	/* I/O now possible (4.2 BSD) */
#define SIGPWR		30	/* Power failure restart (System V) */
#define SIGSYS		31	/* Bad system call */
#define SIGRTMIN	32
#define SIGRTMAX	63
#define SIGMAX		63

/* in this implementation simply use signals as numbers from 1 to SIGMAX */

/*! signal sets and operations on them */
#define SIGSET_ELEM(SIG)	(SIG /(8*sizeof(uint)))
#define SIGSET_POS(SIG)		(SIG %(8*sizeof(uint)))
#define SIGSET_ELEMS		(SIGSET_ELEM(SIGMAX) + 1)

typedef struct sigset
{
	uint  set [SIGSET_ELEMS];
}
sigset_t;

/* information sent with signals (pointer OR integer) */
typedef union sigval
{
	int    sival_int;
	       /* Integer signal value */

	void  *sival_ptr;
	       /* Pointer signal value */
}
sigval_t;

/* information provided with signal, as parameter in signal catching function */
typedef struct siginfo
{
	int       si_signo;
		  /* Signal number */

	int	  si_code;
		  /* Signal code */

	int	  si_errno;
		  /* If non-zero, an errno value associated with this signal */

	pid_t	  si_pid;
		  /* Sending process ID (actually thread id) */

	sigval_t  si_value;
		  /* Signal value */

/* Not implemented:
	uid_t	  si_uid;
		  -* Real user ID of sending process *-
	void	 *si_addr;
		  -* Address of faulting instruction *-
	int	  si_status;
		  -* Exit value or signal */
}
siginfo_t;

/* constants for si_code */
enum {
	SI_USER		= 1,
	SI_KERNEL	= 2,
	SI_QUEUE	= 4,
	SI_TIMER	= 8,
	SI_MESGQ	= 16,
};

/* define signal handling through sigaction */
typedef struct sigaction
{
	union {
		void (*sa_handler)(int);
		        /* Pointer to a signal-catching function or one of the
			 * SIG_IGN or SIG_DFL */

		void (*sa_sigaction)(siginfo_t *);
		        /* Pointer to a signal-catching function;
		         * POSIX standard interface with three parameters not
			 * supported:
		         *     void (*sa_sigaction)(int, siginfo_t *, void *);
		         */
	}
	__sigaction_handler;
	#define sa_handler     __sigaction_handler.sa_handler
	#define sa_sigaction   __sigaction_handler.sa_sigaction

	sigset_t  sa_mask;
		  /* Set of signals to be blocked during execution of the signal
		   * handling function */

	int       sa_flags;
		  /* Special flags */
}
sigaction_t;

/* valid constants for sa_handler */
#define	SIG_ERR		(void *) -1
#define	SIG_DFL		(void *) 0
#define	SIG_IGN		(void *) 1
#define	SIG_HOLD	(void *) 2

/* Bits in `sa_flags'  */
#define SA_SIGINFO	4
/* Invoke signal-catching function with three arguments instead of one */

//#define SA_RESTART	0x10000000 /* Restart syscall on signal return */

//#define SA_NODEFER	0x40000000
/* Don't automatically block the signal when its handler is being executed */

//#define SA_RESETHAND	0x80000000 /* Reset to SIG_DFL on entry to handler */

/* Some aliases for the SA_ constants.  */
//#define SA_NOMASK	SA_NODEFER
//#define SA_ONESHOT	SA_RESETHAND
//#define SA_STACK	SA_ONSTACK

/* Values for the HOW argument to `sigprocmask'.  */
#define SIG_BLOCK	0	/* Block signals.  */
#define SIG_UNBLOCK	1	/* Unblock signals.  */
#define SIG_SETMASK	2	/* Set the set of blocked signals.  */

/* how to react on signal */
typedef struct sigevent
{
	int	  sigev_notify;		/* Notification type */
	int	  sigev_signo;		/* Signal number */
	sigval_t  sigev_value;		/* Signal value */

	union {
		struct _sigev_thread_
		{
			void (*_function)(sigval_t);
			       /* Notification function */

			pthread_attr_t  *_attribute;
					/* Notification attributes (if new
					 * thread is created to handle signal)*/
		}
		_sigev_thread;

		pid_t _tid;
	}
	_sigev_un;
	#define sigev_notify_function	_sigev_un._sigev_thread._function
	#define sigev_notify_attributes	_sigev_un._sigev_thread._attribute
	#define sigev_notify_thread_id	_sigev_un._tid;
}
sigevent_t;

/* signal notification type */
enum {
	SIGEV_NONE = 1,	/* No asynchronous notification is delivered when the
			   event of interest occurs */
	SIGEV_SIGNAL,	/* A queued signal, with an application-defined value,
			   is generated when the event of interest occurs */
	SIGEV_THREAD,	/* A notification function is called to perform
			   notification */
	SIGEV_THREAD_ID	/* Send signal to specific thread (Linux spec.) */
};

/* sigset manipulation */
static inline int sigemptyset(sigset_t *set)
{
	if (!set)
		return FALSE;

	int i;
	for (i = 0; i < SIGSET_ELEMS; i++)
		set->set[i] = 0;

	return 0;
}
static inline int sigfillset(sigset_t *set)
{
	if (!set)
		return FALSE;

	int i;
	for (i = 0; i < SIGSET_ELEMS; i++)
		set->set[i] = (uint) -1;

	return 0;
}
static inline int sigaddset(sigset_t *set, int sig)
{
	if (!set || sig < 1 || sig > SIGMAX)
		return FALSE;

	set->set[SIGSET_ELEM(sig)] |= 1 << SIGSET_POS(sig);

	return 0;
}
static inline int sigdelset(sigset_t *set, int sig)
{
	if (!set || sig < 1 || sig > SIGMAX)
		return FALSE;

	set->set[SIGSET_ELEM(sig)] &= ~(1 << SIGSET_POS(sig));

	return 0;
}

/* not standard, but useful functions */

/* is signal "sig" masked in "set" */
static inline int sigtestset(sigset_t *set, int sig)
{
	if (!set || sig < 1 || sig > SIGMAX)
		return FALSE;

	if (set->set[SIGSET_ELEM(sig)] &(1 << SIGSET_POS(sig)))
		return TRUE;
	else
		return FALSE;
}

/* add sigset "add" to sigset "set" */
static inline int sigaddsets(sigset_t *set, sigset_t *add)
{
	int i;
	for (i = 0; i < SIGSET_ELEMS; i++)
		set->set[i] |= add->set[i];

	return 0;
}

/* clear mask "del" in sigset "set" */
static inline int sigdelsets(sigset_t *set, sigset_t *del)
{
	int i;
	for (i = 0; i < SIGSET_ELEMS; i++)
		set->set[i] &= ~del->set[i];

	return 0;
}

/* is "set" empty set */
static inline int sigtestemptyset(sigset_t *set)
{
	int i;
	uint eset = 0;

	for (i = 0; i < SIGSET_ELEMS; i++)
		eset += set->set[i];

	return eset == 0;
}
