/*! Generate segmentation fault */

#include <stdio.h>
#include <api/prog_info.h>

/* detect memory faults (qemu do not detect segment violations!) */

int segm_fault(char *argv[])
{
	unsigned int *p;
	unsigned int i, j=0;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 segm_fault_PROG_HELP);

	printf("Before segmentation fault\n");

	for (i = 2; i < 32; i++)
	{
		p = (unsigned int *)(1 << i);
		printf("[%x]=%d\n", p, *p);
		j+= *p;
	}

	printf("After expected segmentation fault, j=%d\n", j);

	return 0;
}
