/*! Hello world program */

#include <stdio.h>
#include <lib/string.h>

char PROG_HELP[] = "Print 'Hello world'.";

int hello_world ( char *args[] )
{
	printf ( "Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP );

	printf ( "Hello World!\n" );

/* strtok test
	char test[] = "Test453bla23xxx999xxx0";
	char *tok = strtok ( test, "0123456789" );

	while ( tok )
	{
		printf ( "token: %s\n", tok );
		tok = strtok ( NULL, "0123456789" );
	}
*/
	return 0;
}
