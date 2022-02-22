/*! Keyboard api testing */

#include <stdio.h>
#include <time.h>
#include <api/prog_info.h>

int keyboard()
{
	int key;
	timespec_t t = {.tv_sec = 0, .tv_nsec = 100000000};

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 keyboard_PROG_HELP);

	do {
		if ((key = getchar()))
			printf("Got: %c(%d)\n", key, key);
		else
			nanosleep(&t, NULL);
	}
	while (key != '.');

	printf("End of keyboard test\n");

	return 0;
}
