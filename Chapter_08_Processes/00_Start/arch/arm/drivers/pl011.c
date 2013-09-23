/*! Print on console using ARM PrimeCell UART (PL011) [simple mode!] */

#ifdef PL011

#include "pl011.h"
#include <types/io.h>
#include <lib/string.h>
#include <types/basic.h>
#include <arch/device.h>
#include <arch/interrupt.h>
#include <kernel/errno.h>

static int  uart_init (uint flags, void *params, device_t *dev );
static int  uart_destroy ( uint flags, void *params, device_t *dev );
static void uart_interrupt_handler ( int irq_num, void *dev );
static void uart_write ( arch_uart_t *up );
static int  uart_send ( void *data, size_t size, uint flags, device_t *dev );
static void uart_read ( arch_uart_t *up );
static int  uart_recv ( void *data, size_t size, uint flags, device_t *dev );


/*! Init console */
static int uart_init ( uint flags, void *params, device_t *dev )
{
	volatile unsigned int *uart_ibrd, *uart_fbrd;
	volatile unsigned int *uart_lcr_h, *uart_cr, *uart_imsc;
	arch_uart_t *up;

	ASSERT ( dev );

	up = dev->params;

	uart_ibrd = (unsigned int *) (up->base + UART_IBRD);
	uart_fbrd = (unsigned int *) (up->base + UART_FBRD);
	uart_lcr_h = (unsigned int *) (up->base + UART_LCR_H);
	uart_cr = (unsigned int *) (up->base + UART_CR);
	uart_imsc = (unsigned int *) (up->base + UART_IMSC);

	*uart_cr = 0;

	*uart_ibrd = UART_IBRD_V;
	*uart_fbrd = UART_FBRD_V;
	*uart_lcr_h = UART_LCR_H_WL; /* not using FIFO! */
	*uart_imsc = UART_IMSC_RXIM | UART_IMSC_TXIM;

	*uart_cr = UART_CR_RXE | UART_CR_TXE | UART_CR_UARTEN;

	return 0;
}

/*! Disable UART device */
static int uart_destroy ( uint flags, void *params, device_t *dev )
{
	volatile unsigned int *uart_cr;
	arch_uart_t *up;

	ASSERT ( dev );

	up = dev->params;

	uart_cr = (unsigned int *) (up->base + UART_CR);
	*uart_cr = 0; /* disable UART */

	return 0;
}

/*! Interrupt handler for UART device */
static void uart_interrupt_handler ( int irq_num, void *dev )
{
	volatile unsigned int *uart_icr;
	device_t *uart = dev;
	arch_uart_t *up;

	ASSERT ( dev );

	up = uart->params;

	/* not checking for errors; just get received character to software
	 * buffer and send next from send buffer if previous is sent */

	/* clear interrupts */
	uart_icr = (unsigned int *) (up->base + UART_ICR);
	*uart_icr = UART_IMSC_RXIM | UART_IMSC_TXIM;

	/* "synchronize" UART with software buffer */
	uart_write ( uart->params );
	uart_read ( uart->params );
}


/*! If there is data in software buffer, send it to UART */
static void uart_write ( arch_uart_t *up )
{
	volatile unsigned int *uart_dr;
	volatile unsigned int *uart_fr;

	ASSERT ( up );

	uart_dr = (unsigned int *) (up->base + UART_DR);
	uart_fr = (unsigned int *) (up->base + UART_FR);

	/* If UART is ready to transmit and software buffer is not empty */
	while ( up->outsz > 0 && ( (*uart_fr) & UART_FR_TXFF ) == 0 )
	{
		*uart_dr = (unsigned int) up->outbuff[up->outf];
		INC_MOD ( up->outf, up->outbufsz );
		up->outsz--;
	}
}

/*! Send data to uart */
static int uart_send ( void *data, size_t size, uint flags, device_t *dev )
{
	arch_uart_t *up;
	uint8 *d, pchar;
	console_cmd_t *cmd;

	ASSERT ( dev );

	up = dev->params;
	d = data;

	if ( dev->flags & DEV_TYPE_CONSOLE )
	{
		cmd = data;

		switch ( cmd->cmd )
		{
			case CONSOLE_PRINT:
				d = (uint8 *) &cmd->cd.print.text[0];
				break;

			case CONSOLE_CLEAR:
			case CONSOLE_GOTOXY:
				/* go to new line */
				pchar = '\n';
				d = &pchar;
				size = 1;
				break;

			default:
				return EXIT_FAILURE;
		}
	}
	/*else {
		send raw data;
	}*/

	/* first, copy to software buffer */
	while ( size > 0 && up->outsz < up->outbufsz )
	{
		if ( *d == 0 && flags == CONSOLE_PRINT )
		{
			size = 0;
			break;
		}
		up->outbuff[up->outl] = *d++;
		INC_MOD ( up->outl, up->outbufsz );
		up->outsz++;
		size--;
	}

	/* second, copy from software buffer to uart */
	uart_write ( up );

	return size; /* 0 if all sent, otherwise not send part length */
}

/*! Read data from UART to software buffer */
static void uart_read ( arch_uart_t *up )
{
	volatile unsigned int *uart_dr;
	volatile unsigned int *uart_fr;

	ASSERT ( up );

	uart_dr = (unsigned int *) (UART0_BASE + UART_DR);
	uart_fr = (unsigned int *) (UART0_BASE + UART_FR);

	/* While UART is not empty and software buffer is not full */
	while ( ( (*uart_fr) & UART_FR_RXFE ) == 0 && up->insz < up->inbufsz )
	{
		up->inbuff[up->inl] = *uart_dr;
		INC_MOD ( up->inl, up->inbufsz );
		up->insz++;
	}
}

/*! Read from UART (using software buffer) */
static int uart_recv ( void *data, size_t size, uint flags, device_t *dev )
{
	arch_uart_t *up;
	uint8 *d;
	int i;

	ASSERT ( dev );

	up = dev->params;

	/* first, copy from uart to software buffer */
	uart_read ( up );

	/* second, copy from software buffer to data */
	d = data;
	i = 0;
	while ( i < size && up->insz > 0 )
	{
		d[i] = up->inbuff[up->inf];
		INC_MOD ( up->inf, up->inbufsz );
		up->insz--;
		i++;
	}

	return i; /* bytes read */
}

/*! uart0 device & parameters */
static uint8 uart0_inbuf[BUFFER_SIZE];
static uint8 uart0_outbuf[BUFFER_SIZE];

static arch_uart_t uart0_params = (arch_uart_t)
{
	.base = UART0_BASE,
	.inbuff = uart0_inbuf,
	.inbufsz=BUFFER_SIZE, .inf = 0, .inl = 0, .insz = 0,
	.outbuff = uart0_outbuf,
	.outbufsz=BUFFER_SIZE, .outf = 0, .outl = 0, .outsz = 0
};

/*! uart0 as device_t -----------------------------------------------------*/
device_t pl011_dev = (device_t)
{
	.dev_name =	"PL011",
	.irq_num = 	IRQ_OFFSET + UART0IRQL,
	.irq_handler =	uart_interrupt_handler,

	.init =		uart_init,
	.destroy =	uart_destroy,
	.send =		uart_send,
	.recv =		uart_recv,

	.flags = 	DEV_TYPE_SHARED | DEV_TYPE_CONSOLE,
	.params = 	(void *) &uart0_params
};

#endif /* PL011 */
