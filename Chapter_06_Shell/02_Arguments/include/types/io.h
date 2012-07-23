/*! Basic data types and constants */
#pragma once

#include <types/basic.h>

/*! console commands (simple terminal emulation commands) ------------------- */
#define CONSOLE_KERNEL	( 1 << 0 )
#define CONSOLE_USER	( 1 << 1 )

#define CONSOLE_PRINT	( 1 << 2 )
#define CONSOLE_CLEAR	( 1 << 3 )
#define CONSOLE_GOTOXY	( 1 << 4 )

#define CONSOLE_RAW	( 1 << 5 )	/* raw input/output */
#define CONSOLE_ASCII	( 1 << 6 )	/* send/get only ascii */

#define CONSOLE_MAXLEN	200

/* using console for print requires data with following format */
typedef struct _console_cmd_t_
{
	int cmd;	/* *PRINT, *CLEAR or *GOTOXY */

	union _console_data_
	{
		struct print_string
		{
			int   attr;
			char  text[CONSOLE_MAXLEN];
		}
		print;

		struct _gotoxy_
		{
			int x;
			int y;
		}
		gotoxy;
	}
	cd;
}
console_cmd_t;

/*! flags for operation on devices and other objects */
#define O_CREAT			( 1 << 0 )
#define O_EXCL			( 1 << 1 )
#define O_NONBLOCK		( 1 << 2 )
#define O_RDONLY		( 1 << 3 )
#define O_WRONLY		( 1 << 4 )
#define O_RDWR			( O_RDONLY | O_WRONLY )

#define DEV_OPEN		( 1 << 28 )
#define DEV_TYPE_SHARED		( 1 << 28 )
#define DEV_TYPE_NOTSHARED	( 1 << 29 )
#define DEV_TYPE_CONSOLE	( 1 << 30 )	/* "console mode" = text mode */

/*! limits for name lengths of named system objects */
#define	PATH_MAX		255
#define	NAME_MAX		255

#define DEV_NAME_LEN		32
