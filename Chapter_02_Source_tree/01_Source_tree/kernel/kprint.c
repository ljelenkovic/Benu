/*! Formated printing on console */
#define _K_PRINT_C_

#include <kernel/kprint.h>
#include <arch/print.h>

int kconsole_init()
{
	return arch_console_init();
}

int kconsole_print_word(char *word)
{
	return arch_console_print_word(word);
}