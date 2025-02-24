/*! Generate segmentation fault */

#include <stdio.h>
#include <api/prog_info.h>
#include <arch/interrupt.h>
#include <arch/processor.h>

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

	printf("\nInterrupt test >>>\n");

	arch_register_interrupt_handler(SOFTWARE_INTERRUPT, test1);
	arch_register_interrupt_handler(SOFTWARE_INTERRUPT, test2);

	raise_interrupt(SOFTWARE_INTERRUPT);

	printf("Interrupt test <<<\n\n");

	return 0;
}
