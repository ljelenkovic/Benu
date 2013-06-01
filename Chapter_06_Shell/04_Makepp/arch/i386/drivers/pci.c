/*! PCI */
#ifdef PCI

#include "pci.h"

#include "../io.h"
#include "../interrupt.h"
#include <arch/device.h>
#include <kernel/errno.h>

/*!
 * Read 32 bit information (register) from PCI Configuration Space
 */
uint32 pciConfigRead ( uint32 bus, uint32 slot, uint32 func, uint32 offset )
{
	unsigned long address;

	address = (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc)
		   | ((uint32) 0x80000000) ;

	/* write out the address */
	outl ( 0xCF8, address );

	/* read in the data */
	return inl (0xCFC);
}

int pci_init ()
{
	uint bus, dev, data;
	uint dev_id, vendor;
	uint status, command;
	uint class_code, subclass, prog_IF, rev_id;
	uint bist, header_type, lat_timer, cache_ln_sz;

	/* list pci devices */
	for ( bus = 0; bus < 256; bus++ )
		for ( dev = 0; dev < 32; dev++ )
		{
			data = pciConfigRead ( bus, dev, 0, 0x00 );
			vendor = data & 0x0000ffff;

			if ( vendor != 0xffff )
			{
				dev_id = data >> 16;

				data = pciConfigRead ( bus, dev, 0, 0x4 );
				command = data & 0x0000ffff;
				status = data >> 16;

				data = pciConfigRead ( bus, dev, 0, 0x8 );
				rev_id = data & 0x000000ff;
				prog_IF = (data >> 8) & 0x000000ff;
				subclass = (data >> 16) & 0x000000ff;
				class_code = (data >> 24) & 0x000000ff;

				data = pciConfigRead ( bus, dev, 0, 0xc );
				cache_ln_sz = data & 0x000000ff;
				lat_timer = (data >> 8) & 0x000000ff;
				header_type = (data >> 16) & 0x000000ff;
				bist = (data >> 24) & 0x000000ff;

				kprintf ( "\nPCI device on: %d %d\n", bus, dev );
				kprintf ( "dev_id = %d\n", dev_id );
				kprintf ( "vendor = %d\n", vendor );
				kprintf ( "status = %d\n", status );
				kprintf ( "command = %d\n", command );
				kprintf ( "class_code = %d\n", class_code );
				kprintf ( "subclass = %d\n", subclass );
				kprintf ( "prog_IF = %d\n", prog_IF );
				kprintf ( "rev_id = %d\n", rev_id );
				kprintf ( "bist = %d\n", bist );
				kprintf ( "header_type = %d\n", header_type );
				kprintf ( "lat_timer = %d\n", lat_timer );
				kprintf ( "cache_ln_sz = %d\n", cache_ln_sz );

				data = pciConfigRead ( bus, dev, 0, 0x10 );
				kprintf ( "bar0 = %x\n", data );
				data = pciConfigRead ( bus, dev, 0, 0x14 );
				kprintf ( "bar1 = %x\n", data );

			}
		}

	return 0;
}

#endif /* PCI */
