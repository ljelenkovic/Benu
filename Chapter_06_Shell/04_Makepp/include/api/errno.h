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

#ifndef _KERNEL_

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

#ifndef ASSERT

#include <arch/processor.h>

#define ASSERT(expr)						\
do if (!(expr))						\
{								\
	printf("[BUG:%s:%d]\n", __FILE__, __LINE__);		\
	halt();						\
} while (0)
#endif /* ASSERT */

#ifndef LOG
#define LOG(LEVEL, format, ...)	\
printf("[" #LEVEL ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif /* LOG */

#else /* !DEBUG */

#define ASSERT_ERRNO_AND_RETURN(expr, errnum)
#define ASSERT(expr)
#define LOG(LEVEL, format, ...)

#endif /* DEBUG */

#endif /* _KERNEL_ */
