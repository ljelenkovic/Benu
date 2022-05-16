/*! Time managing functions */
#define _K_TIME_C_

#include "time.h"

#include <kernel/kprint.h>
#include <kernel/errno.h>
#include <arch/time.h>
#include <arch/processor.h>

/*! Initialize time management subsystem */
int k_time_init()
{
	arch_timer_init();

	return EXIT_SUCCESS;
}

/*!
 * Get current time
 * \param clockid Clock to use
 * \param time Pointer where to store time
 */
int kclock_gettime(clockid_t clockid, timespec_t *time)
{
	ASSERT(time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC));

	arch_get_time(time);

	return EXIT_SUCCESS;
}

/*!
 * Set current time
 * \param clockid Clock to use
 * \param time Time to set
 */
int kclock_settime(clockid_t clockid, timespec_t *time)
{
	ASSERT(time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC));

	arch_set_time(time);

	return EXIT_SUCCESS;
}

/*! Interface to programs --------------------------------------------------- */

/*!
 * Get current time
 * \param clockid Clock to use
 * \param time Pointer where to store time
 * \return status (0 if successful, -1 otherwise)
 */
int sys__clock_gettime(clockid_t clockid, timespec_t *time)
{
	int retval;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(
		time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC),
		EINVAL
	);

	retval = kclock_gettime(clockid, time);

	SYS_EXIT(retval, retval);
}

/*!
 * Set current time
 * \param clockid Clock to use
 * \param time Time to set
 * \return status
 */
int sys__clock_settime(clockid_t clockid, timespec_t *time)
{
	int retval;

	SYS_ENTRY();

	ASSERT_ERRNO_AND_EXIT(
		time && (clockid==CLOCK_REALTIME || clockid==CLOCK_MONOTONIC),
		EINVAL
	);

	retval = kclock_settime(clockid, time);

	SYS_EXIT(retval, retval);
}
