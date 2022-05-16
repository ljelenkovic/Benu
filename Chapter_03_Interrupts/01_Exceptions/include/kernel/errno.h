/*! Error numbers, macros, ... (for kernel and arch layer) */
#pragma once

#include <types/errno.h>
#include <kernel/kprint.h>
#include <arch/processor.h>

/*! macros that are removed in release versions - depend on DEBUG */
#ifdef DEBUG

/* Debugging outputs (includes files and line numbers!) */
#define LOG(LEVEL, format, ...)	\
kprintf("[" #LEVEL ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/* Critical error - print it and stop */
#define ASSERT(expr)	do if (!(expr)) { LOG(BUG, ""); halt(); } while (0)

#else /* !DEBUG */

#define ASSERT(expr)
#define LOG(LEVEL, format, ...)

#endif /* DEBUG */

/*! macros that are not removed in release versions - don't depend on DEBUG */
/* Debugging outputs (includes files and line numbers!) */
#define log(LEVEL, format, ...)	\
kprintf("[" #LEVEL ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/* Critical error - print it and stop */
#define assert(expr)	do if (!(expr)) { log(BUG, ""); halt(); } while (0)
