/*! Time managing functions */
#pragma once

#include <kernel/time.h>

/*! interface to kernel */

int k_time_init();
int kclock_gettime(clockid_t clockid, timespec_t *time);
int kclock_settime(clockid_t clockid, timespec_t *time);
