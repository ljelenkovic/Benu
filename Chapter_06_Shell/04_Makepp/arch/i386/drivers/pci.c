/*! PCI */
#ifdef PCI

/* Implemented according to: http://wiki.osdev.org/PCI */

#include "pci.h"

#include "../io.h"
#include "../interrupt.h"
#include <arch/device.h>
#include <kernel/errno.h>
#include <kernel/memory.h>
#include <lib/string.h>

static int pci_devs = 5;
static pci_dev_t *pci_dev = NULL;


static uint32 pciConfigRead(uint32 bus,uint32 slot,uint32 func,uint32 offset);
static void pci_dump();


int pci_init ()
{
	uint bus, dev, func = 0, data;
	int i = 0;

	pci_dev = kmalloc ( sizeof (pci_dev_t) * pci_devs );
	memset ( pci_dev, 0, sizeof (pci_dev_t) * pci_devs );

	/* list pci devices */
	for ( bus = 0; bus < 256; bus++ )
	{
		for ( dev = 0; dev < 32; dev++ )
		{
			data = pciConfigRead ( bus, dev, func, 0x00 );

			if ( (data & 0x0000ffff) == 0xffff )
				continue;

			pci_dev[i].bus = bus;
			pci_dev[i].dev = dev;
			pci_dev[i].func = func;
			pci_dev[i].dummy = 0;

			pci_dev[i].vendor_id = data & 0x0000ffff;
			pci_dev[i].device_id = data >> 16;

			data = pciConfigRead ( bus, dev, func, 0x4 );
			pci_dev[i].command = data & 0x0000ffff;
			pci_dev[i].status = data >> 16;

			data = pciConfigRead ( bus, dev, func, 0x8 );
			pci_dev[i].revision_id = data & 0x000000ff;
			pci_dev[i].prog_if = (data >> 8) & 0x000000ff;
			pci_dev[i].subclass = (data >> 16) & 0x000000ff;
			pci_dev[i].class_code = (data >> 24) & 0x000000ff;

			data = pciConfigRead ( bus, dev, func, 0xC );
			pci_dev[i].cache_line_size = data & 0x000000ff;
			pci_dev[i].latency_timer = (data >> 8) & 0x000000ff;
			pci_dev[i].header_type = (data >> 16) & 0x000000ff;
			pci_dev[i].bist = (data >> 24) & 0x000000ff;

			pci_dev[i].bar0 = pciConfigRead (bus, dev, func, 0x10);
			pci_dev[i].bar1 = pciConfigRead (bus, dev, func, 0x14);
			pci_dev[i].bar2 = pciConfigRead (bus, dev, func, 0x18);
			pci_dev[i].bar3 = pciConfigRead (bus, dev, func, 0x1C);
			pci_dev[i].bar4 = pciConfigRead (bus, dev, func, 0x20);
			pci_dev[i].bar5 = pciConfigRead (bus, dev, func, 0x24);

			data = pciConfigRead ( bus, dev, func, 0x28 );
			pci_dev[i].cardbus_cis = data;

			data = pciConfigRead ( bus, dev, func, 0x2C );
			pci_dev[i].subsystem_vendor_id = data & 0x0000ffff;
			pci_dev[i].subsystem_id = data >> 16;

			data = pciConfigRead ( bus, dev, func, 0x30 );
			pci_dev[i].exp_ROM_base_addr = data;

			data = pciConfigRead ( bus, dev, func, 0x34 );
			pci_dev[i].capabilities = data & 0x000000ff;

			data = pciConfigRead ( bus, dev, func, 0x3c );
			pci_dev[i].interrupt_line = data & 0x000000ff;
			pci_dev[i].interrupt_pin = (data >> 8) & 0x000000ff;
			pci_dev[i].min_grant = (data >> 16) & 0x000000ff;
			pci_dev[i].max_latency = (data >> 24) & 0x000000ff;

			i++;
			ASSERT ( i < pci_devs );
		}
	}

	pci_devs = i;

	kprintf ( "Devices: %d\n", pci_devs );

	pci_dump ();

	return 0;
}

static void pci_dump ()
{
	int i = 0;

	/* list pci devices */
	kprintf ( "========================= PCI devices =================="
		  "==========" );
	for ( i = 0; i < pci_devs; i++ )
	{
		kprintf ( "\nPCI device on (bus dev func): %d %d %d\n",
			  pci_dev[i].bus, pci_dev[i].dev, pci_dev[i].func );
		kprintf ( "-------------------------------------\n" );

		kprintf ( "device_id = %x\n", pci_dev[i].device_id );
		kprintf ( "vendor_id = %x\n", pci_dev[i].vendor_id );

		kprintf ( "status = %d\n", pci_dev[i].status );
		kprintf ( "command = %d\n", pci_dev[i].command );

		kprintf ( "class_code = %d\n", pci_dev[i].class_code );
		kprintf ( "subclass = %d\n", pci_dev[i].subclass );
		kprintf ( "prog_if = %d\n", pci_dev[i].prog_if );
		kprintf ( "revision_id = %d\n", pci_dev[i].revision_id );

		kprintf ( "bist = %d\n", pci_dev[i].bist );
		kprintf ( "header_type = %d\n", pci_dev[i].header_type );
		kprintf ( "latency_timer = %d\n", pci_dev[i].latency_timer );
		kprintf ( "cache_line_size = %d\n", pci_dev[i].cache_line_size );

		kprintf ( "bar0-bar5= %x %x %x %x %x %x\n",
			  pci_dev[i].bar0, pci_dev[i].bar1, pci_dev[i].bar2,
			  pci_dev[i].bar3, pci_dev[i].bar4, pci_dev[i].bar5 );

		kprintf ( "cardbus_cis = %d\n", pci_dev[i].cardbus_cis );

		kprintf ( "subsystem_id = %d\n", pci_dev[i].subsystem_id );
		kprintf ( "subsystem_vendor_id = %d\n", pci_dev[i].subsystem_vendor_id );

		kprintf ( "capabilities = %d\n", pci_dev[i].capabilities );

		kprintf ( "max_latency = %d\n", pci_dev[i].max_latency );
		kprintf ( "min_grant = %d\n", pci_dev[i].min_grant );
		kprintf ( "interrupt_pin = %d\n", pci_dev[i].interrupt_pin );
		kprintf ( "interrupt_line = %d\n", pci_dev[i].interrupt_line );
	}

	kprintf ( "======================================================="
  		  "==========\n\n" );
}

int pci_get_dev ( uint32 device_id, uint32 vendor_id )
{
	int i = 0;

	for ( i = 0; i < pci_devs; i++ )
		if ( pci_dev[i].device_id == device_id &&
			pci_dev[i].vendor_id == vendor_id )
			return i;
	return -1;
}

int pci_get_base_addr ( int dev, int num, uint32 *addr, int *type )
{
	uint32 baddr;

	ASSERT ( dev >= 0 && dev < pci_devs && addr && type );
	switch ( num )
	{
	case 0 :
		baddr = pci_dev[dev].bar0;
		break;
	case 1 :
		baddr = pci_dev[dev].bar1;
		break;
	case 2 :
		baddr = pci_dev[dev].bar2;
		break;
	case 3 :
		baddr = pci_dev[dev].bar3;
		break;
	case 4 :
		baddr = pci_dev[dev].bar4;
		break;
	case 5 :
		baddr = pci_dev[dev].bar5;
		break;
	default:
		return -EINVAL;
	}

	if ( baddr & 1 )
	{
		*addr = baddr & 0xfffffffc;
		*type = 0;
	}
	else {
		*addr = baddr & 0xfffffff0;
		*type = 1;
	}

	return 0;
}

/*!
 * Read 32 bit information (register) from PCI Configuration Space
 */
static uint32 pciConfigRead(uint32 bus, uint32 slot, uint32 func, uint32 offset)
{
	unsigned long address;

	address = (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc)
		   | ((uint32) 0x80000000) ;

	/* write out the address */
	outl ( 0xCF8, address );

	/* read in the data */
	return inl (0xCFC);
}

#endif /* PCI */
