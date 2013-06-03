/*! PCI */
#ifdef PCI

#pragma once

#include <arch/device.h>
#include <types/io.h>

struct _pci_dev_t_
{
	uint8  bus, dev, func, dummy;
	/*     |      0-7     |      8-15    |     16-23    |     24-31    |*/
	uint16		vendor_id,			device_id;
	uint16		command,			status;
	uint8	revision_id,	 prog_if,	subclass,	class_code;
	uint8	cache_line_size, latency_timer,	header_type,	bist;
	uint32				bar0;
	uint32				bar1;
	uint32				bar2;
	uint32				bar3;
	uint32				bar4;
	uint32				bar5;
	uint32				cardbus_cis;
	uint16		subsystem_vendor_id,		subsystem_id;
	uint32				exp_ROM_base_addr;
	uint8	capabilities,	 reserved2;		uint16 reserved1;
	uint32				reserved3;
	uint8	interrupt_line,	 interrupt_pin,	min_grant,	max_latency;
}
__attribute__ ((packed));

typedef struct _pci_dev_t_ pci_dev_t;

int pci_init ();
int pci_get_dev ( uint32 device_id, uint32 vendor_id );
int pci_get_base_addr ( int dev, int num, uint32 *addr, int *type );



#endif /* PCI */
