/*! Time managing functions */
#pragma once

#include <types/time.h>
#include <types/signal.h>

/*! interface to threads (via syscall) */
int sys__clock_gettime(void *p);
int sys__clock_settime(void *p);
int sys__clock_nanosleep(void *p);

int sys__timer_create(void *p);
int sys__timer_delete(void *p);
int sys__timer_settime(void *p);
int sys__timer_gettime(void *p);
