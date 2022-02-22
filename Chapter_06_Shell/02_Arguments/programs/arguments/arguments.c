/*! Print program arguments */

#include <stdio.h>
#include <api/prog_info.h>

int arguments(char *argv[])
{
	int i;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 arguments_PROG_HELP);

	printf("Program arguments: ");
	for (i = 0; argv && argv[i]; i++)
		printf("[%s] ", argv[i]);
	printf("\n");

	return 0;
}
