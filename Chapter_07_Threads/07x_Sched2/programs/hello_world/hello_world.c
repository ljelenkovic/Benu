/*! Hello world program */

#include <stdio.h>

char PROG_HELP[] = "Print 'Hello world'.";

int hello_world(char *args[])
{
	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	printf("Hello World!\n");

#if 0	/* test escape sequence */
	printf("\x1b[20;40H" "Hello World at 40, 20!\n");
#endif

	return 0;
}
