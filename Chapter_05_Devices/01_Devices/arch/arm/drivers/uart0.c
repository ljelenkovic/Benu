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

static void uart0_putchar ( char c );
static int uart0_init ();
static int uart0_clear ();
static int uart0_gotoxy ( int x, int y );
static int uart0_printf ( int attr, char *text );
static int uart0_send ( void *data, size_t size, uint flags, device_t *dev );


/*!
 * Send character on uart0
 * \param c Single character
 */
static void uart0_putchar ( char c )
{
	volatile unsigned int *uart_dr = (unsigned int *) UART0_DR;
	volatile unsigned int *uart_fr = (unsigned int *) UART0_FR;

	/* Wait for UART to become ready to transmit */
	while ( (*uart_fr) & (1 << 5) )
		;

	/* Transmit char */
	*uart_dr = (unsigned int) c;

}

#if 1 /* test UART receive irqs with echo */
#include <arch/interrupt.h>
/*!
 * Get character from uart0
 */
static int uart0_getchar ()
{
	volatile unsigned int *uart_dr = (unsigned int *) UART0_DR;
	volatile unsigned int *uart_fr = (unsigned int *) UART0_FR;

	if ( (*uart_fr) & (1 << 4) ) /* empty */
		return 0;
	else
		return *uart_dr;
}

static void uart_echo ()
{
	uart0_putchar ( uart0_getchar() );
}

/*! Init console with interrupt enable */
void uart0_echo_test_start ()
{
	volatile unsigned int *uart_imsc = (unsigned int *) UART0_IMSC;

	/* enable RXIM interrupt (interrupt on data receive) */
	*uart_imsc =  1 << 4;

	/* register UART interrupt */
	arch_register_interrupt_handler ( IRQ_OFFSET + UART0IRQL, uart_echo,
					  NULL );

	/* enable UART interrupt in VIC */
	arch_irq_enable ( IRQ_OFFSET + UART0IRQL );
}
#endif

/*! Init console */
int uart0_init ( uint flags, void *params, device_t *dev )
{
	return 0;
}

/*!
 * Clear console
 * (just go to new line - send \n on serial port)
 */
int uart0_clear ()
{
	uart0_putchar ( '\n' );
	return 0;
}

/*!
 * Move cursor to specified location
 * (just go to new line - send \n on serial port)
 */
static int uart0_gotoxy ( int x, int y )
{
	uart0_putchar ( '\n' );
	return 0;
}

/*!
 * Print text string on console, starting at current cursor position
 * \param attr Attributes to apply (ignored)
 * \param text String to print
 */
static int uart0_printf ( int attr, char *text )
{
	if ( text == NULL )
		return 0;


	while ( *text != '\0' ) /* Loop until end of string */
		uart0_putchar ( *text++ );

	return strlen ( text );
}

/*! Device wrapper for console */
static int uart0_send ( void *data, size_t size, uint flags, device_t *dev )
{
	int retval;
	console_cmd_t *cmd;

	if ( dev->flags & DEV_TYPE_CONSOLE )
	{
		cmd = data;

		switch ( cmd->cmd )
		{
			case CONSOLE_PRINT:
				retval = uart0_printf ( cmd->cd.print.attr,
							&cmd->cd.print.text[0]);
				break;

			case CONSOLE_CLEAR:
				retval = uart0_clear ();
				break;

			case CONSOLE_GOTOXY:
				retval = uart0_gotoxy (
					cmd->cd.gotoxy.x, cmd->cd.gotoxy.y );
				break;

			default:
				retval = EXIT_FAILURE;
				break;
		}
	}
	else {
		retval = EXIT_FAILURE;
	}

	return retval;
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
	.recv =		NULL,

	.flags = 	DEV_TYPE_SHARED | DEV_TYPE_CONSOLE,
	.params = 	(void *) &uart0_dev
};

#endif /* UART0 */
