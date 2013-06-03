/*! RTL8139 Realtek Fast Ethernet */
#ifdef RTL8139

#include "rtl8139.h"

#include "../io.h"
#include "../interrupt.h"
#include "pci.h"

#include <arch/device.h>
#include <kernel/errno.h>
#include <kernel/memory.h>
#include <lib/string.h>

/* FIXME: move to kmalloc */
static char rx_buffer[8192+16]; // declare the local buffer space (8k + header)


static int rtl8139_init ( uint flags, void *params, device_t *dev )
{
	arch_rtl8139_t *p;
	uint16 iobase;

	ASSERT ( dev && dev->params );

	p = dev->params;

	if ( p->pci_dev != -1 )
	{
		/* already initialized or error? */
		return -EBUSY;
	}

	p->pci_dev = pci_get_dev ( p->device_id, p->vendor_id );
	if ( p->pci_dev == -1 )
		return -ENODEV;

	if ( pci_get_base_addr ( p->pci_dev, 0, &p->iobase, &p->iobase_type ) )
		return -EINVAL;

	iobase = (uint16) p->iobase;
	/* Turning on the RTL8139 */
	outb ( iobase + 0x52, 0x0 );

	/* Software Reset! */
	outb ( iobase + 0x37, 0x10);
	while ( ( inb ( iobase + 0x37 ) & 0x10 ) != 0) { }

	/* Init Receive buffer */
	outl ( iobase + 0x30, (unsigned long) rx_buffer );
	// send dword memory location to RBSTART (0x30)

	/* Set IMR + ISR */
	outw ( iobase + 0x3C, 0x0005 ); // Sets the TOK and ROK bits high

	/* Configuring receive buffer (RCR) */
	outb ( iobase + 0x44, 0xf | (1 << 7) );
	// (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

	/* Enable Receive and Transmitter */
	outb ( iobase + 0x37, 0x0C ); // Sets the RE and TE bits high

	return 0;
}

static int rtl8139_destroy ( uint flags, void *params, device_t *dev )
{
	return 0;
}

static int rtl8139_send ( void *data, size_t size, uint flags, device_t *dev )
{
	return 0;
}

static int rtl8139_recv ( void *data, size_t size, uint flags, device_t *dev )
{
	return 0;
}

static void rtl8139_interrupt_handler ( int irq_num, void *device )
{
}


/*! RTL8139 device & parameters */
static arch_rtl8139_t rtl8139_params = (arch_rtl8139_t)
{
	.device_id = 0x10ec,
	.vendor_id = 0x8139,
	.pci_dev = -1,
	.iobase = 0,
	.iobase_type = 0
};

/*! RTL8139 as device_t */
device_t rtl8139 = (device_t)
{
	.dev_name =	"RTL8139",

	.irq_num = 	0,
	.irq_handler =	rtl8139_interrupt_handler,

	.init =		rtl8139_init,
	.destroy =	rtl8139_destroy,
	.send =		rtl8139_send,
	.recv =		rtl8139_recv,

	.flags = 	0,
	.params = 	&rtl8139_params
};


#endif /* RTL8139 */
