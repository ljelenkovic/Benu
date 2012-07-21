/*! Error numbers, macros, ... (for kernel and arch layer) */
#pragma once

int sys__set_errno ( void *p );
int sys__get_errno ( void *p );
int sys__get_errno_ptr ( void *p );

#ifdef _KERNEL_

#include <types/errno.h>

#include <kernel/thread.h>
#include <kernel/kprint.h>
#include <arch/processor.h>


/* error number is defined per thread (saved in thread descriptor) */

/* set errno */
#define SET_ERRNO(ENUM)		kthread_set_errno (NULL, ENUM)

/* return from syscall, with error number provided;
 * return 0 if no error, -1 otherwise */
#define EXIT(ENUM)				\
do {						\
	int ec = ENUM;				\
	SET_ERRNO ( ec );			\
	return ( ec ? EXIT_FAILURE : 0 );	\
}						\
while (0)

/* return from syscall, with return value and error number provided */
#define EXIT2(ENUM,RETVAL)	do { SET_ERRNO(ENUM); return RETVAL; } while (0)


#ifdef DEBUG

/*! Debugging outputs (includes files and line numbers!) */
#define LOG(LEVEL, format, ...)	\
kprintf ( "[" #LEVEL ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/*! Critical error - print it and stop */
#define ASSERT(expr)	do if ( !( expr ) ) { LOG ( BUG, ""); halt(); } while(0)

/* assert and return (inter kernel calls) */
#define ASSERT_AND_RETURN_ERRNO(expr, errnum) \
do {	if ( !( expr ) )			\
	{					\
		LOG ( ASSERT, "");		\
		return errnum;			\
	}					\
} while(0)

/* assert and return from syscall */
#define ASSERT_ERRNO_AND_EXIT(expr, errnum)	\
do {	if ( !( expr ) )			\
	{					\
		LOG ( ASSERT, "");		\
		EXIT ( errnum );		\
	}					\
} while(0)

#else /* !DEBUG */

#define ASSERT(expr)
#define ASSERT_ERRNO_AND_EXIT(expr, errnum)
#define ASSERT_AND_RETURN_ERRNO(expr, errnum)
#define LOG(LEVEL, format, ...)

#endif /* DEBUG */

#endif /* _KERNEL_ */