/*! Memory management */
#pragma once

#include <types/basic.h>

/*! interface to programs */
int sys__sysinfo ( char *buffer, size_t buf_size );

extern inline void *k_mem_init ( void *segment, size_t size );
extern inline void *kmalloc ( size_t size );
extern inline int kfree ( void *chunk );

struct _kobject_t_; typedef struct _kobject_t_ kobject_t;
