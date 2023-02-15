/*! Basic data types and constants */
#pragma once

#include <types/basic.h>

/*! Escape characters */
#define ESCAPE			27
#define ESC_COLOR_RED		31
#define ESC_COLOR_GREEN		32
#define ESC_COLOR_WHITE		37
#define ESC_COLOR_DEFAULT	39
/*
 * look in arch/i386/drivers/vga_text.c for supported escape sequences for that
 * device (vga_text)
 */

#define CONSOLE_PRINT	(1 << 1)
#define CONSOLE_RAW	(1 << 2)	/* raw input/output */
#define CONSOLE_ASCII	(1 << 3)	/* send/get only ascii */
#define CONSOLE_MAXLEN	200

/*! Console interface */
typedef struct _console_t_
{
	int (*init)(int flags);
	int (*print)(char *text);
}
console_t;
