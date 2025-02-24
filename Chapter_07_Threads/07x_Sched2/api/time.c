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

/*!
 * Suspend thread until given time elapses
 * \param clockid Clock to use
 * \param flags flags (TIMER_ABSTIME)
 * \param request Suspend duration
 * \param remain Remainder time if interrupted during suspension
 * \return status
 */
int clock_nanosleep(clockid_t clockid, int flags, timespec_t *request,
		      timespec_t *remain)
{
	ASSERT_ERRNO_AND_RETURN(request && (clockid == CLOCK_REALTIME ||
				  clockid == CLOCK_MONOTONIC), EINVAL      );

	return sys__clock_nanosleep(clockid, flags, request, remain);
}

/*!
 * Suspend thread until given time elapses
 * \param request Suspend duration
 * \param remain Remainder time if interrupted during suspension
 * \return status
 */
int nanosleep(timespec_t *request, timespec_t *remain)
{
	ASSERT_ERRNO_AND_RETURN(request, EINVAL);

	return sys__clock_nanosleep(CLOCK_REALTIME, 0, request, remain);
}


/*! Timer ------------------------------------------------------------------- */

/*!
 * Create new timer
 * \param clockid	Clock used in timer
 * \param evp		Timer expiration action
 * \param timerid	Timer descriptor is returned in this variable
 * \return status	0 for success
 */
int timer_create(clockid_t clockid, sigevent_t *evp, timer_t *timer)
{
	ASSERT_ERRNO_AND_RETURN(evp && timer && (clockid == CLOCK_REALTIME
				  || clockid == CLOCK_MONOTONIC), EINVAL   );

	return sys__timer_create(clockid, evp, timer);
}

/*!
 * Delete timer
 * \param timerid	Timer descriptor (user descriptor)
 * \return status	0 for success
 */
int timer_delete(timer_t *timer)
{
	ASSERT_ERRNO_AND_RETURN(timer, EINVAL);

	return sys__timer_delete(timer);
}

/*!
 * Arm/disarm timer
 * \param timerid	Timer descriptor (user descriptor)
 * \param flags		Various flags (TIMER_ABSTIME)
 * \param value		Set timer values (it_value+it_period)
 * \param ovalue	Where to store time to next timer expiration (+period)
 * \return status	0 for success
 */
int timer_settime(timer_t *timer, int flags, itimerspec_t *value,
		     itimerspec_t *ovalue)
{
	ASSERT_ERRNO_AND_RETURN(timer, EINVAL);

	return sys__timer_settime(timer, flags, value, ovalue);
}

/*!
 * Get timer expiration time
 * \param timerid	Timer descriptor (user descriptor)
 * \param value		Where to store time to next timer expiration (+period)
 * \return status	0 for success
 */
int timer_gettime(timer_t *timer, itimerspec_t *value)
{
	ASSERT_ERRNO_AND_RETURN(timer && value, EINVAL);

	return sys__timer_gettime(timer, value);
}
