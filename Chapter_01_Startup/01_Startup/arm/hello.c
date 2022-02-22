/* Print a string on serial port 0 */

#define UART0_BASE	0x101f1000
#define UART0_FR	(UART0_BASE + 0x18)
#define UART0_DR	(UART0_BASE + 0x00)

void print_uart0(const char *s)
{
	volatile unsigned int *uart_dr = (unsigned int *) UART0_DR;
	volatile unsigned int *uart_fr = (unsigned int *) UART0_FR;

	while (*s != '\0') /* Loop until end of string */
	{
		/* Wait for UART to become ready to transmit */
		while ((*uart_fr) &(1 << 5))
			;

		/* Transmit char */
		*uart_dr = (unsigned int)(*s);

		s++; /* Next char */
	}
}

void print_hello()
{
	 print_uart0("Hello world!\n");
}
