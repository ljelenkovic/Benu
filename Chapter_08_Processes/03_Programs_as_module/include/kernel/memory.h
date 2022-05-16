/*! Memory management */
#pragma once

/*! interface to threads (via syscall) */
int sys__sysinfo(void *p);

#ifdef _KERNEL_ /* (for kernel and arch layer) */

#include <types/basic.h>

void *k_mem_init(void *segment, size_t size);
void *kmalloc(size_t size);
int kfree(void *chunk);

struct _kobject_t_; typedef struct _kobject_t_ kobject_t;
struct _kprog_t_; typedef struct _kprog_t_ kprog_t;

#endif /* _KERNEL_ */
