/*! PCI */
#ifdef PCI

/* Implemented according to: http://wiki.osdev.org/PCI */

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
	uint bus, dev, func = 0, data;
	uint dev_id, vendor;
	uint status, command;
	uint class_code, subclass, prog_IF, rev_id;
	uint bist, header_type, lat_timer, cache_ln_sz;
	uint max_lat, min_grant, int_pin, int_line;

	/* list pci devices */
	kprintf ( "========================= PCI devices =================="
		  "==========" );
	for ( bus = 0; bus < 256; bus++ )
		for ( dev = 0; dev < 32; dev++ )
		{
			data = pciConfigRead ( bus, dev, func, 0x00 );
			vendor = data & 0x0000ffff;

			if ( vendor != 0xffff )
			{
				kprintf ( "\nPCI device on (bus dev func)"
					  ": %d %d %d\n", bus, dev, func );
				kprintf ( "-------------------------------"
					  "------\n" );

				dev_id = data >> 16;
				kprintf ( "\tdev_id = %d\t", dev_id );
				kprintf ( "\tvendor = %d\n", vendor );

				data = pciConfigRead ( bus, dev, func, 0x4 );
				command = data & 0x0000ffff;
				status = data >> 16;
				kprintf ( "\tstatus = %d\t", status );
				kprintf ( "\tcommand = %d\n", command );

				data = pciConfigRead ( bus, dev, func, 0x8 );
				rev_id = data & 0x000000ff;
				prog_IF = (data >> 8) & 0x000000ff;
				subclass = (data >> 16) & 0x000000ff;
				class_code = (data >> 24) & 0x000000ff;
				kprintf ( "cl.code = %d\t", class_code );
				kprintf ( "subclass = %d\t", subclass );
				kprintf ( "prog_IF = %d\t", prog_IF );
				kprintf ( "rev_id = %d\n", rev_id );

				data = pciConfigRead ( bus, dev, func, 0xc );
				cache_ln_sz = data & 0x000000ff;
				lat_timer = (data >> 8) & 0x000000ff;
				header_type = (data >> 16) & 0x000000ff;
				bist = (data >> 24) & 0x000000ff;
				kprintf ( "bist = %d\t", bist );
				kprintf ( "head.type = %d\t", header_type );
				kprintf ( "lat_timer = %d\t", lat_timer );
				kprintf ( "cache_ln_sz = %d\n", cache_ln_sz );

				data = pciConfigRead ( bus, dev, func, 0x10 );
				if ( data & 1 )
					kprintf ( "bar0 = %x (I/O)\n",
						  data & 0xfffffffc );
				else
					kprintf ( "bar0 = %x (mem)\n",
						  data & 0xfffffff0 );

				data = pciConfigRead ( bus, dev, func, 0x14 );
				if ( data & 1 )
					kprintf ( "bar1 = %x (I/O)\n",
						  data & 0xfffffffc );
				else
					kprintf ( "bar1 = %x (mem)\n",
						  data & 0xfffffff0 );

				data = pciConfigRead ( bus, dev, func, 0x3c );
				max_lat = data & 0x000000ff;
				min_grant = (data >> 8) & 0x000000ff;
				int_pin = (data >> 16) & 0x000000ff;
				int_line = (data >> 24) & 0x000000ff;
				kprintf ( "max_lat = %d\t", max_lat );
				kprintf ( "min_grant = %d\t", min_grant );
				kprintf ( "int_pin = %d\t", int_pin );
				kprintf ( "int_line = %d\n", int_line );
			}
		}

	kprintf ( "======================================================="
  		  "==========\n\n" );

	return 0;
}

#endif /* PCI */
