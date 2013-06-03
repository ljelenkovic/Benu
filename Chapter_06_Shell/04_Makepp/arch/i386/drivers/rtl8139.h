/*! RTL8139 Realtek Fast Ethernet */
#ifdef RTL8139

#pragma once

#include <arch/device.h>
#include <types/io.h>


typedef struct _arch_rtl8139_t_
{
	uint32  device_id;
	uint32  vendor_id;
	int     pci_dev;
	uint32  iobase;
	int     iobase_type;
}
arch_rtl8139_t;


#endif /* RTL8139 */
