/*! Error numbers, macros, ... */
#pragma once

#include <types/errno.h>

extern int _errno;

static inline int set_errno(int error_number)
{
	return _errno = error_number;
}
static inline int get_errno()
{
	return _errno;
}

#ifdef DEBUG

#include <api/stdio.h>

/* assert and return */
#define ASSERT_ERRNO_AND_RETURN(expr, errnum)			\
do {								\
	if (!(expr))					\
	{							\
		warn("%s:%d::ASSERT\n", __FILE__, __LINE__);	\
		set_errno(errnum);				\
		return EXIT_FAILURE;				\
	 }							\
} while (0)

#else /* !DEBUG */

#define ASSERT_ERRNO_AND_RETURN(expr, errnum)

#endif /* DEBUG */
