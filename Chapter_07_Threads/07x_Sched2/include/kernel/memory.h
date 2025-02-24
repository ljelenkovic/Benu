/*! Memory management */
#pragma once

#include <types/basic.h>

/*! interface to threads */
int sys__sysinfo(char *buffer, size_t buf_size, char **param);

#ifdef _KERNEL_ /* (for kernel and arch layer) */

void *k_mem_init(void *segment, size_t size);
void *kmalloc(size_t size);
int kfree(void *chunk);

struct _kobject_t_; typedef struct _kobject_t_ kobject_t;

#endif /* _KERNEL_ */
