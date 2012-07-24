/*! Formated printing on console (using 'device_t' interface) */
#define _K_PRINT_C_

#include <kernel/kprint.h> /* shares kprint with arch layer */

#include "device.h"
#include <lib/string.h>

void *k_stdout; /* initialized in startup.c */

/*! Formated output to console (lightweight version of 'printf') */
int kprintf ( char *format, ... )
{
	console_cmd_t cmd;
	size_t size;

	cmd.cmd = CONSOLE_PRINT;
	cmd.cd.print.attr = CONSOLE_KERNEL;

	size = vssprintf ( &cmd.cd.print.text[0], CONSOLE_MAXLEN, &format );

	return k_device_send ( &cmd, size, 0, k_stdout );
}