/*! Basic data types and constants */
#pragma once

#include <types/basic.h>

/*! console commands (simple terminal emulation commands) ------------------- */
#define CONSOLE_KERNEL	( 1 << 0 )
#define CONSOLE_USER	( 1 << 1 )
#define CONSOLE_MAXLEN	200

/*! Console interface (used in kernel mode) */
typedef struct _console_t_
{
	int (*init) ( int flags );
	int (*clear) ();
	int (*gotoxy) ( int x, int y );
	int (*print) ( int attr, char *text );
}
console_t;
