/*! Error numbers, macros, ... (for kernel and arch layer) */
#pragma once

#include <types/errno.h>
#include <api/errno.h>

#include <kernel/kprint.h>
#include <arch/processor.h>


/* error number is defined globally (saved in variable _errno) */

/* set errno */
#define SET_ERRNO(ENUM)		set_errno(ENUM)


/* syscall enter procedures (mark IE flag and disable interrupts) */
#define SYS_ENTRY()		int __FUNCTION__ ## ei = set_interrupts(FALSE)

/* return from syscall with return value: restore interrupts, don't set errno */
#define SYS_RETURN(RETVAL)						\
do { set_interrupts(__FUNCTION__ ## ei); return RETVAL; } while (0)

/* return from syscall with return value: restore interrupts and set errno */
#define SYS_EXIT(ENUM,RETVAL)				\
do { SET_ERRNO(ENUM); SYS_RETURN(RETVAL); } while (0)

/* Important: SYS_ENTRY must be in same level or higher in code, since both
 * SYS_EXIT and SYS_RETURN use variable defined by SYS_ENTRY */


/*! macros that are removed in release versions - depend on DEBUG */
#ifdef DEBUG

/* Debugging outputs (includes files and line numbers!) */
#define LOG(LEVEL, format, ...)	\
kprintf("[" #LEVEL ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/* Critical error - print it and stop */
#define ASSERT(expr)	do if (!(expr)) { LOG(BUG, ""); halt(); } while (0)

/* assert and return (inter kernel calls) */
#define ASSERT_AND_RETURN_ERRNO(expr, errnum)		\
do { if (!(expr)) { LOG(ASSERT, ""); return errnum; } } while (0)

/* assert and return from syscall */
#define ASSERT_ERRNO_AND_EXIT(expr, errnum)		\
do if (!(expr)) { LOG(ASSERT, ""); SYS_EXIT(errnum, EXIT_FAILURE); } while (0)

#else /* !DEBUG */

#define ASSERT(expr)
#define ASSERT_ERRNO_AND_EXIT(expr, errnum)
#define ASSERT_AND_RETURN_ERRNO(expr, errnum)
#define LOG(LEVEL, format, ...)

#endif /* DEBUG */

/*! macros that are not removed in release versions - don't depend on DEBUG */
/* Debugging outputs (includes files and line numbers!) */
#define log(LEVEL, format, ...)	\
kprintf("[" #LEVEL ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/* Critical error - print it and stop */
#define assert(expr)	do if (!(expr)) { log(BUG, ""); halt(); } while (0)

/* assert and return (inter kernel calls) */
#define assert_and_return_errno(expr, errnum)		\
do { if (!(expr)) { log(ASSERT, ""); return errnum; } } while (0)

/* assert and return from syscall */
#define assert_errno_and_exit(expr, errnum)		\
do if (!(expr)) { log(ASSERT, ""); SYS_EXIT(errnum, EXIT_FAILURE); } while (0)

