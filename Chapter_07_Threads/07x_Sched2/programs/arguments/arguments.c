/*! Print program arguments */

#include <stdio.h>

char PROG_HELP[] = "Print given command line arguments.";

int arguments(char *argv[])
{
	int i;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 PROG_HELP);

	printf("Program arguments: ");
	for (i = 0; argv && argv[i]; i++)
		printf("[%s] ", argv[i]);
	printf("\n");

	return 0;
}
