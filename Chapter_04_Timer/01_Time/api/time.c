/*! Time managing functions */

#include <api/time.h>

#include <kernel/time.h>
#include <api/stdio.h>
#include <api/errno.h>

/*! Time -------------------------------------------------------------------- */

/*!
 * Get current time
 * \param clockid Clock to use
 * \param time Pointer where to store time
 * \return status (0 if successful, -1 otherwise)
 */
int clock_gettime(clockid_t clockid, timespec_t *time)
{
	ASSERT_ERRNO_AND_RETURN(time && (clockid == CLOCK_REALTIME ||
				  clockid == CLOCK_MONOTONIC), EINVAL      );

	return sys__clock_gettime(clockid, time);
}

/*!
 * Set current time
 * \param clockid Clock to use
 * \param time Time to set
 * \return status
 */
int clock_settime(clockid_t clockid, timespec_t *time)
{
	ASSERT_ERRNO_AND_RETURN(time && (clockid == CLOCK_REALTIME ||
				  clockid == CLOCK_MONOTONIC), EINVAL      );

	return sys__clock_settime(clockid, time);
}
