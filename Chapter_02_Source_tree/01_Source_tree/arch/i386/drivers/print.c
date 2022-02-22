/*! Print on console using video memory */

#include <arch/print.h>
#include <lib/string.h>

#define VIDEO	((char *) 0x000B8000) /* video memory address */
#define COLS	80 /* number of characters in a column */
#define ROWS	25 /* number of characters in a row */
#define ATTR	7  /* font: white char on black bacground */

static volatile char *video = VIDEO;

/*! Init console */
int arch_console_init()
{
	/* erase screen (set blank screen) */
	memset((void *) video, 0, COLS * ROWS * 2);

	return 0;
}

/*!
 * Print single word on console in new line
 * \param word Word to print (word must be shorter than 80 characters)
 */
int arch_console_print_word(char *word)
{
	static int row = 0;
	int i;

	if (word == NULL)
		return -1;

	/* erase line */
	memset((void *)(video + row * COLS * 2), 0, COLS * 2);

	/* print word on new line */
	for (i = 0; word[i] != 0 && i < COLS; i++)
	{
		video [ row * COLS * 2 + i * 2 ]     = word[i];
		video [ row * COLS * 2 + i * 2 + 1 ] = ATTR;
	}

	if (row < ROWS)
	{
		row++;
	}
	else {
		memmove((void *) video, (void *)(video + COLS * 2),
			  ROWS * COLS * 2);
		memset((void *) video + row * COLS * 2, 0, COLS * 2);
	}

	return 0;
}
