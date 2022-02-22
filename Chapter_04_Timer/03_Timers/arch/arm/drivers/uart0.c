/*! Print on console using ARM PrimeCell UART (PL011) [very simple mode!] */

#ifdef UART0

#include <types/io.h>
#include <lib/string.h>
#include <types/basic.h>
#include <ARCH/config.h>

#define UART0_FR	(UART0_BASE + 0x18)
#define UART0_DR	(UART0_BASE + 0x00)

static void uart0_putchar(char c);
static int uart0_init(int flags);
static int uart0_printf(char *text);


/*!
 * Send character on uart0
 * \param c Single character
 */
static void uart0_putchar(char c)
{
	volatile unsigned int *uart_dr = (unsigned int *) UART0_DR;
	volatile unsigned int *uart_fr = (unsigned int *) UART0_FR;

	/* Wait for UART to become ready to transmit */
	while ((*uart_fr) &(1 << 5))
		;

	/* Transmit char */
	*uart_dr = (unsigned int) c;

}

/*! Init console */
int uart0_init(int flags)
{
	return 0;
}

/*!
 * Print text string on console, starting at current cursor position
 * \param text String to print
 */
static int uart0_printf(char *text)
{
	if (text == NULL)
		return 0;


	while (*text != '\0') /* Loop until end of string */
		uart0_putchar(*text++);

	return strlen(text);
}


/*! uart0 as console */
console_t uart0 = (console_t)
{
	.init	= uart0_init,
	.print	= uart0_printf
};

static int _do_nothing_()
{
	return 0;
}

/*! dev_null */
console_t dev_null = (console_t)
{
	.init	= (void *) _do_nothing_,
	.print	= (void *) _do_nothing_
};

#endif /* UART0 */
