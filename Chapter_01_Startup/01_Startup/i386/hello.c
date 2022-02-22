/*! Print on console using video memory */

#define VIDEO	((volatile char *) 0x000B8000) /* address of video memory  */
#define COLS	80 /* number of characters in a column */
#define ROWS	25 /* number of characters in a row */
#define ATTR	7  /* font: white char on black bacground */

/*! Print "Hello world!" */
void print_hello()
{
	int i;
	char hello[] = "Hello World!";
	volatile char *video = VIDEO;

	/* erase screen (set blank screen) */
	for (i = 0; i < COLS * ROWS; i++)
		video[ i * 2 ] = video[ i * 2 + 1 ] = 0;

	/* print "Hello World!" on first line */
	for (i = 0; i < 13; i++)
	{
		video[ i * 2 ]     = hello[i];
		video[ i * 2 + 1 ] = ATTR;
	}

	return;
}
