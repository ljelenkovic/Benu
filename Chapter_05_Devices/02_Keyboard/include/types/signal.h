/*! Signals - just structures and constants required for timers */
#pragma once

#include <types/basic.h>

/* information included in sigevent (pointer OR integer) */
typedef union sigval
{
	int    sival_int;
	       /* Integer signal value */

	void  *sival_ptr;
	       /* Pointer signal value */
}
sigval_t;

/* how to react on signal (generated through timers) */
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

			void  *_attribute;
				/* Notification attributes (if new
				 * thread is created to handle signal)*/
		}
		_sigev_thread;

		id_t _tid;
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
