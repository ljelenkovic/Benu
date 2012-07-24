/*! Memory management */
#pragma once

#include <types/basic.h>

extern inline void *k_mem_init ( void *segment, size_t size );
extern inline void *kmalloc ( size_t size );
extern inline int kfree ( void *chunk );
