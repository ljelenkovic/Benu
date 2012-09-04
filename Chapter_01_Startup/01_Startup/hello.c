/*! Print on console using video memory */

#define VIDEO	( (volatile char *) 0x000B8000 ) /* video memory address */
#define COLS	80 /* number of characters in a column */
#define ROWS	25 /* number of characters in a row */
#define ATTR	7  /* font: white char on black bacground */

/*! Print "Hello world!" */
void print_hello ()
{
	int i;
	char hello[] = "Hello World!";

	/* erase screen (set blank screen) */
	for ( i = 0; i < COLS * ROWS; i++ )
		*( VIDEO + i * 2 ) = *( VIDEO + i * 2 + 1 ) = 0;

	/* print "Hello World!" on first line */
	for ( i = 0; i < 13; i++ )
	{
		*( VIDEO + i * 2 )     = hello[i];
		*( VIDEO + i * 2 + 1 ) = ATTR;
	}

	return;
}
