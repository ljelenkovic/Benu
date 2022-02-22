/*! Formated printing on console */
#define _K_PRINT_C_

#include <kernel/kprint.h> /* shares kprint with arch layer */

#include <lib/string.h>
#include <types/io.h>

console_t *k_stdout; /* initialized in startup.c */

/*! Formated output to console (lightweight version of 'printf') */
int kprintf(char *format, ...)
{
	size_t size;
	char buffer[CONSOLE_MAXLEN];

	k_stdout->print("\x1b[31m"); /* red color for text */

	size = vssprintf(buffer, CONSOLE_MAXLEN, &format);
	k_stdout->print(buffer);

	k_stdout->print("\x1b[39m"); /* default color for text */

	return size;
}
