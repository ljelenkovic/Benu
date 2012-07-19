/*! Thread management */
#pragma once

#include <lib/list.h>

/*! Thread queue (only structure required to be visible outside thread.c) */
typedef struct _kthread_q_
{
	list_t  q;		/* queue implementation in list.h/list.c */
	/* uint flags; */	/* various flags, e.g. sort order */
}
kthread_q;
