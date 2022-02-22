/*! Print on console using ARM PrimeCell UART (PL011) [very simple mode!] */

#ifdef UART0

#include <types/io.h>
#include <lib/string.h>
#include <types/basic.h>
#include <arch/device.h>
#include <kernel/errno.h>
#include <ARCH/config.h>

#define UART0_FR	(UART0_BASE + 0x18)
#define UART0_DR	(UART0_BASE + 0x00)
#define UART0_IMSC	(UART0_BASE + 0x38)

static void uart0_putchar(char c);
static int uart0_getchar();
static int uart0_init();
static int uart0_send(void *data, size_t size, uint flags, device_t *dev);
static int uart0_recv(void *data, size_t size, uint flags, device_t *dev);


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

/*!
 * Get character from uart0
 */
static int uart0_getchar()
{
	volatile unsigned int *uart_dr = (unsigned int *) UART0_DR;
	volatile unsigned int *uart_fr = (unsigned int *) UART0_FR;

	if ((*uart_fr) &(1 << 4)) /* empty */
		return -1;

	return *uart_dr;
}

/*! Init console */
int uart0_init(uint flags, void *params, device_t *dev)
{
	return 0;
}

/*! Device wrapper for console */
static int uart0_send(void *data, size_t size, uint flags, device_t *dev)
{
	char *text = data;

	if (dev->flags & DEV_TYPE_CONSOLE)
	{
		if (text == NULL)
			return 0;

		while (*text != '\0') /* Loop until end of string */
			uart0_putchar(*text++);

		return strlen(text);
	}
	else {
		return EXIT_FAILURE;
	}
}

/*!
 * Get character from uart0
 */
static int uart0_recv(void *data, size_t size, uint flags, device_t *dev)
{
	int c;

	if (!data || size < 1)
		return -1;

	c = uart0_getchar();

	if (c == -1)	/* no new data */
		return 0;

	*((uint *) data) = c;

	return 1;
}

/*! Get status */
static int uart_status(uint flags, device_t *dev)
{
	int rflags = 0;
	volatile unsigned int *uart_fr = (unsigned int *) UART0_FR;

	if (!((*uart_fr) &(1 << 4))) /* Have something to read? */
		rflags |= DEV_IN_READY;

	if (!((*uart_fr) &(1 << 5))) /* Is UART ready to transmit? */
		rflags |= DEV_OUT_READY;

	return DEV_IN_READY;
}

/*! uart as device_t -----------------------------------------------------*/
device_t uart0_dev = (device_t)
{
	.dev_name =	"UART0",
	.irq_num = 	-1,
	.irq_handler =	NULL,

	.init =		uart0_init,
	.destroy =	NULL,
	.send =		uart0_send,
	.recv =		uart0_recv,
	.status = 	uart_status,

	.flags = 	DEV_TYPE_SHARED | DEV_TYPE_CONSOLE,
	.params = 	(void *) &uart0_dev
};

#endif /* UART0 */
