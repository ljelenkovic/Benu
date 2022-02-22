/*! Printing on stdout (from kernel) */
#pragma once

#ifdef _KERNEL_

#include <types/io.h>

int kprintf(char *format, ...);

#endif /* _KERNEL_ */
