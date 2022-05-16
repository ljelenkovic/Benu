/*! Basic data types and constants */
#pragma once

#include <types/basic.h>

/*! Escape characters */
#define ESCAPE			27
#define ESC_COLOR_RED		31
#define ESC_COLOR_GREEN		32
#define ESC_COLOR_WHITE		37
#define ESC_COLOR_DEFAULT	39
/*
 * look in arch/i386/drivers/vga_text.c for supported escape sequences for that
 * device (vga_text)
 */

#define CONSOLE_PRINT	(1 << 1)
#define CONSOLE_RAW	(1 << 2)	/* raw input/output */
#define CONSOLE_ASCII	(1 << 3)	/* send/get only ascii */
#define CONSOLE_MAXLEN	200


/*! flags for operation on devices and other objects */
#define O_CREAT			(1 << 0)
#define O_EXCL			(1 << 1)
#define O_NONBLOCK		(1 << 2)
#define O_RDONLY		(1 << 3)
#define O_WRONLY		(1 << 4)
#define O_RDWR			(O_RDONLY | O_WRONLY)

#define DEV_OPEN		(1 << 28)
#define DEV_TYPE_SHARED		(1 << 28)
#define DEV_TYPE_NOTSHARED	(1 << 29)
#define DEV_TYPE_CONSOLE	(1 << 30)	/* "console mode" = text mode */

/*! limits for name lengths of named system objects (as message queues) */
#define	PATH_MAX		255
#define	NAME_MAX		255

#define DEV_NAME_LEN		32


/*! Device status: have new data / can new data be sent to device */
#define DEV_IN_READY		(1 << 0)   /* have data to read from device */
#define DEV_OUT_READY		(1 << 1)   /* can send data to device */
#define DEV_INT_RQ		(1 << 2)   /* device raised interrupt req. */


/*! input/output multiplexing - get IO status */

/*! The following types and flags are from <poll.h> (POSIX) */

/* Each IO is specified by file descriptor index and events to be checked */
struct pollfd {
	int    fd;	/* The following descriptor being polled */
	short  events;	/* The input event flags */
	short  revents;	/* The output event flags */
};

typedef unsigned int nfds_t; //number of file descriptors

/* int poll(struct pollfd fds[], nfds_t nfds, int timeout); */

#define POLLIN		(1<<0)
/* Data other than high-priority data may be read without blocking. */

#define POLLRDNORM	(1<<1)
/* Normal data may be read without blocking. */

#define POLLRDBAND	(1<<2)
/* Priority data may be read without blocking. */

#define POLLPRI		(1<<3)
/* High priority data may be read without blocking. */

#define POLLOUT		(1<<4)
/* Normal data may be written without blocking. */

#define POLLWRNORM	(1<<5)
/* Equivalent to POLLOUT. */

#define POLLWRBAND	(1<<6)
/* Priority data may be written. */

#define POLLERR		(1<<7)
/* An error has occurred (revents only). */

#define POLLHUP		(1<<8)
/* Device has been disconnected (revents only). */

#define POLLNVAL	(1<<9)
/* Invalid fd member (revents only). */

#if 0

int   poll(struct pollfd [], nfds_t, int);

/*
 * The significance and semantics of normal, priority, and high-priority data
 * are file and device-specific.
 *
 * If the value of fd is less than 0, events shall be ignored, and revents shall
 * be set to 0 in that entry on return from poll().
 *
 * In each pollfd structure, poll() shall clear the revents member, except that
 * where the application requested a report on a condition by setting one of the
 * bits of events listed above, poll() shall set the corresponding bit in
 * revents if the requested condition is true. In addition, poll() shall set the
 * POLLHUP, POLLERR, and POLLNVAL flag in revents if the condition is true, even
 * if the application did not set the corresponding bit in events.
 *
 * If none of the defined events have occurred on any selected file descriptor,
 * poll() shall wait at least timeout milliseconds for an event to occur on any
 * of the selected file descriptors. If the value of timeout is 0, poll() shall
 * return immediately. If the value of timeout is -1, poll() shall block until a
 * requested event occurs or until the call is interrupted.
 *
 * Implementations may place limitations on the granularity of timeout
 * intervals. If the requested timeout interval requires a finer granularity
 * than the implementation supports, the actual timeout interval shall be
 * rounded up to the next supported value.
 *
 * The poll() function shall not be affected by the O_NONBLOCK flag.
 *
 * The poll() function shall support regular files, terminal and pseudo-terminal
 * devices, FIFOs, pipes, sockets and  STREAMS-based files. The behavior of
 * poll() on elements of fds that refer to other types of file is unspecified.
 *
 * Regular files shall always poll TRUE for reading and writing.
 *
 * A file descriptor for a socket that is listening for connections shall
 * indicate that it is ready for reading, once connections are available. A file
 * descriptor for a socket that is connecting asynchronously shall indicate that
 * it is ready for writing, once a connection has been established.
 */

Upon successful completion, poll() shall return a non-negative value. A positive
value indicates the total number of file descriptors that have been selected
(that is, file descriptors for which the revents member is non-zero). A value of
0 indicates that the call timed out and no file descriptors have been selected.
Upon failure, poll() shall return -1 and set errno to indicate the error.

ERRORS

The poll() function shall fail if:

[EAGAIN]
The allocation of internal data structures failed but a subsequent request may
succeed.
[EINTR]
A signal was caught during poll().
[EINVAL]
The nfds argument is greater than {OPEN_MAX}

EXAMPLES

#include <stropts.h>
#include <poll.h>
...
struct pollfd fds[2];
int timeout_msecs = 500;
int ret;
    int i;


/* Open STREAMS device. */
fds[0].fd = open("/dev/dev0", ...);
fds[1].fd = open("/dev/dev1", ...);
fds[0].events = POLLOUT | POLLWRBAND;
fds[1].events = POLLOUT | POLLWRBAND;


ret = poll(fds, 2, timeout_msecs);


if (ret > 0) {
    /* An event on one of the fds has occurred. */
    for (i=0; i<2; i++) {
        if (fds[i].revents & POLLWRBAND) {
        /* Priority data may be written on device number i. */
...
        }
        if (fds[i].revents & POLLOUT) {
        /* Data may be written on device number i. */
...
        }
        if (fds[i].revents & POLLHUP) {
        /* A hangup has occurred on device number i. */
...
        }
    }
}

#endif
