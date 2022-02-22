/*! Time managing functions */
#pragma once

#include <types/time.h>
#include <types/signal.h>

int clock_gettime(clockid_t clockid, timespec_t *time);
int clock_settime(clockid_t clockid, timespec_t *time);
int clock_nanosleep(clockid_t clockid, int flags, timespec_t *request,
		      timespec_t *remain);
int nanosleep(timespec_t *request, timespec_t *remain);

int timer_create(clockid_t clockid, sigevent_t *evp, timer_t *timer);
int timer_delete(timer_t *timer);
int timer_settime(timer_t *timer, int flags, itimerspec_t *value,
		   itimerspec_t *ovalue);
int timer_gettime(timer_t *timer, itimerspec_t *value);
