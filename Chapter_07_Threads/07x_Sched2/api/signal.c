/*! Signals */

#include <api/signal.h>

#include <kernel/signal.h>
#include <types/basic.h>
#include <api/stdio.h>
#include <api/errno.h>

/*! Signals ----------------------------------------------------------------- */

/*!
 * Modify thread signal mask
 * \param sig Signal number
 * \param act Signal handling behavior
 * \param oact Where to save old signal behavior
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sigaction(int sig, sigaction_t *act, sigaction_t *oact)
{
	ASSERT_ERRNO_AND_RETURN(sig > 0 && sig <= SIGMAX, EINVAL);

	return sys__sigaction(sig, act, oact);
}

/*!
 * Send signal to thread (from thread)
 * \param pid Thread descriptor (user level descriptor)
 * \param signo Signal number
 * \param sigval Parameter to send with signal
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int sigqueue(pid_t pid, int signo, sigval_t sigval)
{
	ASSERT_ERRNO_AND_RETURN(signo > 0 && signo <= SIGMAX, EINVAL);

	return sys__sigqueue(pid, signo, sigval);
}

/*!
 * Wait for signal
 * \param set Signals thread is waiting for
 * \param info Where to save caught signal
 * \return signal number if signal is caught,
 *         -1 otherwise and appropriate error number is set
 */
int sigwaitinfo(sigset_t *set, siginfo_t *info)
{
	ASSERT_ERRNO_AND_RETURN(set, EINVAL);

	return sys__sigwaitinfo(set, info);
}

/*!
 * Modify thread signal mask
 * \param how How to modify current mask
 * \param set Mask to use in modification
 * \param oset Where to store old mask
 * \return 0 if successful, -1 otherwise and appropriate error number is set
 */
int pthread_sigmask(int how, sigset_t *set, sigset_t *oset)
{
	ASSERT_ERRNO_AND_RETURN(set, EINVAL);

	return sys__pthread_sigmask(how, set, oset);
}
