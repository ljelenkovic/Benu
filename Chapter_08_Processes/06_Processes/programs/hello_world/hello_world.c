/*! Hello world program */

#include <stdio.h>

char PROG_HELP[] = "Print 'Hello world'.";

int hello_world ( char *args[] )
{
	printf ( "Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP );

	printf ( "Hello World!\n" );

	return 0;
}
