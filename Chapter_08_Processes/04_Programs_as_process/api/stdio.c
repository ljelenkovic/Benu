/*! Printing on stdout, reading from stdin */

#include <api/stdio.h>

#include <api/syscall.h>
#include <api/prog_info.h>
#include <api/errno.h>
#include <lib/string.h>
#include <types/basic.h>

static descriptor_t std_desc[MAX_USER_DESCRIPTORS];
static int _stdin, _stdout, _stderr;

/*! Initialize standard descriptors (input, output, error) */
int stdio_init ()
{
	int i;
	for ( i = 0; i < MAX_USER_DESCRIPTORS; i++ )
	{
		std_desc[i].id = 0;
		std_desc[i].ptr = NULL;
	}

	_stdin =  open ( U_STDIN,  O_RDONLY | CONSOLE_ASCII, 0 ); /* 0 */
	_stdout = open ( U_STDOUT, O_WRONLY | CONSOLE_ASCII, 0 ); /* 1 */
	_stderr = open ( U_STDERR, O_WRONLY | CONSOLE_ASCII, 0 ); /* 2 */

	ASSERT_ERRNO_AND_RETURN ( _stdin == 0 && _stdout == 1 && _stderr == 2,
				  ENOTSUP );

	return EXIT_SUCCESS;
}

/*! Open a descriptor */
int open ( char *pathname, int flags, mode_t mode )
{
	descriptor_t desc;
	int i, retval;

	for ( i = 0; i < MAX_USER_DESCRIPTORS; i++ )
		if ( std_desc[i].id == 0 )
			break;

	if ( i == MAX_USER_DESCRIPTORS )
	{
		set_errno ( EMFILE );
		return EXIT_FAILURE;
	}

	retval = syscall ( OPEN, pathname, flags, mode, &desc );

	if ( retval )
		return EXIT_FAILURE;

	std_desc[i].id = desc.id;
	std_desc[i].ptr = desc.ptr;

	return i;
}

/*! Close an descriptor */
int close ( int fd )
{
	int retval;

	if ( 	fd < 0 || fd >= MAX_USER_DESCRIPTORS ||
		!std_desc[fd].id || !std_desc[fd].ptr )
	{
		set_errno ( EBADF );
		return EXIT_FAILURE;
	}

	retval = syscall ( CLOSE, &std_desc[fd] );

	if ( retval )
		return EXIT_FAILURE;

	std_desc[fd].id = 0;
	std_desc[fd].ptr = NULL;

	return EXIT_SUCCESS;
}

/*! Read from device */
ssize_t read ( int fd, void *buf, size_t count )
{
	if ( 	fd < 0 || fd >= MAX_USER_DESCRIPTORS ||
		!std_desc[fd].id || !std_desc[fd].ptr || !buf || !count )
	{
		set_errno ( EBADF );
		return EXIT_FAILURE;
	}

	return syscall ( READ, &std_desc[fd], buf, count );
}

/*! Write from device */
ssize_t write ( int fd, void *buf, size_t count )
{
	if ( 	fd < 0 || fd >= MAX_USER_DESCRIPTORS ||
		!std_desc[fd].id || !std_desc[fd].ptr || !buf || !count )
	{
		set_errno ( EBADF );
		return EXIT_FAILURE;
	}

	return syscall ( WRITE, &std_desc[fd], buf, count );
}

/*! Get input from "standard input" */
inline int get_char ()
{
	int c = 0;

	read ( _stdin, &c, sizeof (int) );

	return c;
}

/*! Erase screen (if supported by stdout device) */
inline int clear_screen ()
{
	console_cmd_t cmd;

	cmd.cmd = CONSOLE_CLEAR;

	return write ( _stdout, &cmd, sizeof (int) );
}

/*! Move cursor to given position (if supported by stdout device) */
inline int goto_xy ( int x, int y )
{
	console_cmd_t cmd;

	cmd.cmd = CONSOLE_GOTOXY;
	cmd.cd.gotoxy.x = x;
	cmd.cd.gotoxy.y = y;

	return write ( _stdout, &cmd, sizeof (int) );
}

/*! Formated output to console (lightweight version of 'printf') */
int printf ( char *format, ... )
{
	console_cmd_t cmd;
	size_t size;

	cmd.cmd = CONSOLE_PRINT;
	cmd.cd.print.attr = CONSOLE_USER;

	size = vssprintf ( &cmd.cd.print.text[0], CONSOLE_MAXLEN, &format );

	return write ( _stdout, &cmd, size );
}

/*! Formated output to error console */
void warn ( char *format, ... )
{
	console_cmd_t cmd;
	size_t size;

	cmd.cmd = CONSOLE_PRINT;
	cmd.cd.print.attr = CONSOLE_USER;

	size = vssprintf ( &cmd.cd.print.text[0], CONSOLE_MAXLEN, &format );

	write ( _stderr, &cmd, size );
}

/*! Change standard input device */
int change_stdin ( char *new_stdin )
{
	close ( _stdin );
	_stdin = open ( new_stdin, O_RDONLY | CONSOLE_ASCII, 0 );

	return EXIT_SUCCESS;
}

/*! Change standard output device */
int change_stdout ( char *new_stdout )
{
	close ( _stdout );
	_stdout = open ( new_stdout, O_WRONLY | CONSOLE_ASCII, 0 );

	return EXIT_SUCCESS;
}
