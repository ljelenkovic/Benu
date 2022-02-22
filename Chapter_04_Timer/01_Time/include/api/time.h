/*! Time managing functions */
#pragma once

#include <types/time.h>

int clock_gettime(clockid_t clockid, timespec_t *time);
int clock_settime(clockid_t clockid, timespec_t *time);
