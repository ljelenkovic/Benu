/*! Time managing functions */
#pragma once

#include <types/time.h>
#include <types/signal.h>

/*! interface to programs */
int sys__clock_gettime(clockid_t clockid, timespec_t *time);
int sys__clock_settime(clockid_t clockid, timespec_t *time);
int sys__clock_nanosleep(clockid_t clockid, int flags,
			   timespec_t *request, timespec_t *remain);

int sys__timer_create(clockid_t clockid, sigevent_t *evp, timer_t *timerid);
int sys__timer_delete(timer_t *timerid);
int sys__timer_settime(timer_t *timerid, int flags,
			 itimerspec_t *value, itimerspec_t *ovalue);
int sys__timer_gettime(timer_t *timerid, itimerspec_t *value);
