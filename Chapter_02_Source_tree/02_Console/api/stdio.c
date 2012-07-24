/*! Printing on stdout, reading from stdin */

#include <api/stdio.h>

#include <lib/string.h>
#include <types/io.h>

console_t *u_stdout, *u_stderr;

/*! Initialize standard descriptors (input, output, error) */
int stdio_init ()
{
	extern console_t U_STDOUT, U_STDERR;

	u_stdout = &U_STDOUT;
	u_stdout->init (0);

	u_stderr = &U_STDERR;
	u_stderr->init (0);

	return 0;
}

/*! Erase screen (if supported by stdout device) */
inline int clear_screen ()
{
	return u_stdout->clear ();
}

/*! Move cursor to given position (if supported by stdout device) */
inline int goto_xy ( int x, int y )
{
	return u_stdout->gotoxy ( x, y );
}

/*! Formated output to console (lightweight version of 'printf') */
int printf ( char *format, ... )
{
	char text[CONSOLE_MAXLEN];

	vssprintf ( text, CONSOLE_MAXLEN, &format );

	return u_stdout->print ( CONSOLE_USER, text );
}

/*! Formated output to error console */
void warn ( char *format, ... )
{
	char text[CONSOLE_MAXLEN];

	vssprintf ( text, CONSOLE_MAXLEN, &format );

	u_stderr->print ( CONSOLE_USER, text );
}
