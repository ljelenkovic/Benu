/*! Formated printing on console */

#include <api/stdio.h>
#include <kernel/kprint.h>

int console_init()
{
	return kconsole_init();
}

int console_print_word(char *word)
{
	return kconsole_print_word(word);
}