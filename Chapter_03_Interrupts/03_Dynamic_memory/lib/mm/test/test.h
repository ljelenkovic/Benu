/*! standalone memory allocator test (stress tests for errors) */
#include <stdio.h>

#define ERROR(format, ...)	\
	printf("[ERROR:%s:%d]" format, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG(level, format, ...)	\
printf("[" #level ":%s:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define ASSERT(expr)					\
do if (!(expr))					\
{							\
	printf("[BUG:%s:%d]\n", __FILE__, __LINE__);	\
	exit(1);					\
} while (0)
