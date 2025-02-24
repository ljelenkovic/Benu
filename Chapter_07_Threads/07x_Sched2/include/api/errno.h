/*! Error numbers, macros, ... */
#pragma once

#include <types/errno.h>
#include <kernel/errno.h>

static inline int set_errno(int error_number)
{
	return sys__set_errno(error_number);
}
static inline int get_errno()
{
	return sys__get_errno();
}

static inline int *get_errno_ptr()
{
	int *error_number_ptr;
	sys__get_errno_ptr(&error_number_ptr);

	return error_number_ptr;
}

/*! errno via macro "_errno" */
#define _errno	(*get_errno_ptr())


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

#include <api/pthread.h>

#define ASSERT(expr)						\
do if (!(expr))						\
{								\
	printf("[BUG:%s:%d]\n", __FILE__, __LINE__);		\
	pthread_exit((void *) EXIT_FAILURE);				\
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
