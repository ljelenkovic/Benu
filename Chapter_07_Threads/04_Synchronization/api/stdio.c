/*! Printing on stdout, reading from stdin */

#include <api/stdio.h>

#include <kernel/device.h>
#include <api/errno.h>
#include <lib/string.h>

static descriptor_t std_desc[MAX_USER_DESCRIPTORS];
static int _stdin, _stdout, _stderr;

/*! Initialize standard descriptors (input, output, error) */
int stdio_init()
{
	int i;
	for (i = 0; i < MAX_USER_DESCRIPTORS; i++)
	{
		std_desc[i].id = 0;
		std_desc[i].ptr = NULL;
	}

	_stdin =  open(U_STDIN,  O_RDONLY | CONSOLE_ASCII, 0); /* 0 */
	_stdout = open(U_STDOUT, O_WRONLY | CONSOLE_ASCII, 0); /* 1 */
	_stderr = open(U_STDERR, O_WRONLY | CONSOLE_ASCII, 0); /* 2 */

	ASSERT_ERRNO_AND_RETURN(_stdin == 0 && _stdout == 1 && _stderr == 2,
				  ENOTSUP);

	return EXIT_SUCCESS;
}

/*! Open a descriptor */
int open(char *pathname, int flags, mode_t mode)
{
	descriptor_t desc;
	int i, retval;

	for (i = 0; i < MAX_USER_DESCRIPTORS; i++)
		if (std_desc[i].id == 0)
			break;

	if (i == MAX_USER_DESCRIPTORS)
	{
		set_errno(EMFILE);
		return EXIT_FAILURE;
	}

	retval = sys__open(pathname, flags, mode, &desc);

	if (retval)
		return EXIT_FAILURE;

	std_desc[i].id = desc.id;
	std_desc[i].ptr = desc.ptr;

	return i;
}

/*! Close an descriptor */
int close(int fd)
{
	int retval;

	if (	fd < 0 || fd >= MAX_USER_DESCRIPTORS ||
		!std_desc[fd].id || !std_desc[fd].ptr)
	{
		set_errno(EBADF);
		return EXIT_FAILURE;
	}

	retval = sys__close(&std_desc[fd]);

	if (retval)
		return EXIT_FAILURE;

	std_desc[fd].id = 0;
	std_desc[fd].ptr = NULL;

	return EXIT_SUCCESS;
}

/*! Read from device */
ssize_t read(int fd, void *buffer, size_t count)
{
	if (	fd < 0 || fd >= MAX_USER_DESCRIPTORS ||
		!std_desc[fd].id || !std_desc[fd].ptr || !buffer || !count)
	{
		set_errno(EBADF);
		return EXIT_FAILURE;
	}

	return sys__read(&std_desc[fd], buffer, count);
}

/*! Write from device */
ssize_t write(int fd, void *buffer, size_t count)
{
	if (	fd < 0 || fd >= MAX_USER_DESCRIPTORS ||
		!std_desc[fd].id || !std_desc[fd].ptr || !buffer || !count)
	{
		set_errno(EBADF);
		return EXIT_FAILURE;
	}

	return sys__write(&std_desc[fd], buffer, count);
}

/*! Get input from "standard input" */
int getchar()
{
	int c = 0;

	read(_stdin, &c, sizeof(int));

	return c;
}

/*! Formated output to console (lightweight version of 'printf') */
int printf(char *format, ...)
{
	char buffer[CONSOLE_MAXLEN];
	size_t size;

	size = vssprintf(buffer, CONSOLE_MAXLEN, &format);

	return write(_stdout, buffer, size);
}

/*! Formated output to error console */
void warn(char *format, ...)
{
	char buffer[CONSOLE_MAXLEN];
	size_t size;

	size = vssprintf(buffer, CONSOLE_MAXLEN, &format);

	write(_stderr, buffer, size);
}

/*!
 * poll - input/output multiplexing
 * \param fds set of file descriptors
 * \param nfds number of file descriptors in fds
 * \param timeout minimum time in ms to wait for any event defined by fds
 *       (in current implementation this parameter is ignored)
 * \return number of file descriptors with changes in 'revents', -1 on errors
 */
int poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
	int i;

	if (!fds || nfds < 1)
	{
		set_errno(EBADF);
		return EXIT_FAILURE;
	}

	for (i = 0; i < nfds; i++)
	{
		if (	fds[i].fd < 0 || fds[i].fd >= MAX_USER_DESCRIPTORS ||
			!std_desc[fds[i].fd].id || !std_desc[fds[i].fd].ptr)
		{
			set_errno(EBADF);
			return EXIT_FAILURE;
		}
	}

	return sys__poll(fds, nfds, timeout, std_desc);
}
