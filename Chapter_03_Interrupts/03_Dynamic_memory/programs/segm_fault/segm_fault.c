/*! Generate segmentation fault */

#include <stdio.h>
#include <api/prog_info.h>
#include <api/malloc.h>
#include <arch/interrupt.h>
#include <arch/processor.h>

#define TEST	2

/* detect memory faults (qemu do not detect segment violations!) */

#if TEST == 1
static void test1(uint irqn)
{
	printf("Interrupt handler routine: irqn=%d\n", irqn);
}
#endif

int segm_fault()
{
#if TEST == 1
	printf("\nInterrupt test >>>\n");

	arch_register_interrupt_handler(SOFTWARE_INTERRUPT, test1);
	arch_register_interrupt_handler(SOFTWARE_INTERRUPT, test1);

	raise_interrupt(SOFTWARE_INTERRUPT);

	printf("Interrupt test <<<\n\n");

#elif TEST == 2
	void *ptr1, *ptr2;

	ptr1 = malloc(1023);
	printf("malloc returned %x(1023)\n", ptr1);

	ptr2 = malloc(123);
	printf("malloc returned %x(123)\n", ptr2);

	if (ptr1)
		free(ptr1);
	if (ptr2)
		free(ptr2);
#else
	unsigned int *p;
	unsigned int i, j=0;

	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 segm_fault_PROG_HELP);

	printf("Before segmentation fault\n");

	for (i = 16; i < 32; i++)
	{
		p = (unsigned int *)(1 << i);
		printf("[%x]=%d\n", p, *p);
		j+= *p;
	}

	printf("After expected segmentation fault, j=%d\n", j);
#endif
	return 0;
}
