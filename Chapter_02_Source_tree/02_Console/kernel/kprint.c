/*! Formated printing on console */
#define _K_PRINT_C_

#include <kernel/kprint.h> /* shares kprint with arch layer */

#include <lib/string.h>
#include <types/io.h>

console_t *k_stdout; /* initialized in startup.c */

/*! Formated output to console (lightweight version of 'printf') */
int kprintf ( char *format, ... )
{
	char text[CONSOLE_MAXLEN];

	vssprintf ( text, CONSOLE_MAXLEN, &format );

	return k_stdout->print ( CONSOLE_KERNEL, text );
}