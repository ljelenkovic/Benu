/*! Formated printing on console (using 'device_t' interface) */
#define _K_PRINT_C_

#include <kernel/kprint.h> /* shares kprint with arch layer */

#include "device.h"
#include <lib/string.h>

void *k_stdout; /* initialized in startup.c */

/*! Formated output to console (lightweight version of 'printf') */
int kprintf(char *format, ...)
{
	size_t size;
	char buffer[CONSOLE_MAXLEN];

	k_device_send("\x1b[31m", 6, 0, k_stdout); /* red color for text */

	size = vssprintf(buffer, CONSOLE_MAXLEN, &format);
	k_device_send(buffer, size, 0, k_stdout);

	k_device_send("\x1b[39m", 6, 0, k_stdout); /* default color for text */

	return size;
}
