/*! Generate segmentation fault */

#include <stdio.h>
#include <api/prog_info.h>
#include <arch/interrupt.h>
#include <arch/processor.h>
#include <api/malloc.h>

static void test1(uint irqn)
{
	printf("Interrupt handler routine 1: irqn=%d\n", irqn);
}
static void test2(uint irqn)
{
	printf("Interrupt handler routine 2: irqn=%d\n", irqn);
}

int interrupts()
{
	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 interrupts_PROG_HELP);

	printf("\nDynamic memory allocation test >>>\n");
	void *ptr1, *ptr2;

	ptr1 = malloc(1023);
	printf("malloc returned %x(1023)\n", ptr1);

	ptr2 = malloc(123);
	printf("malloc returned %x(123)\n", ptr2);

	if (ptr1)
		free(ptr1);
	if (ptr2)
		free(ptr2);

	printf("Dynamic memory allocation test <<<\n");


	printf("\nInterrupt test >>>\n");

	arch_register_interrupt_handler(SOFTWARE_INTERRUPT, test1);
	arch_register_interrupt_handler(SOFTWARE_INTERRUPT, test2);

	raise_interrupt(SOFTWARE_INTERRUPT);

	printf("Interrupt test <<<\n\n");

	return 0;
}
