/*! Time managing functions */
#pragma once

#include <types/time.h>

/*! interface to programs */
int sys__clock_gettime(clockid_t clockid, timespec_t *time);
int sys__clock_settime(clockid_t clockid, timespec_t *time);
