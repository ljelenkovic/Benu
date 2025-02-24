/*! Device - common interface  for device drivers */
#pragma once

#include <ARCH/device.h>
#include <types/io.h>

struct _device_t_;
typedef struct _device_t_ device_t;

/*! device_t interface */
struct _device_t_
{
	char    dev_name[DEV_NAME_LEN];
		/* device name */

	int     irq_num;
		/* which IRQ is using? (if any, -1 otherwise) */

	int  (*irq_handler)(int irq_num, void *device);
		/* interrupt handler function (test if device is interrupt
		 * source and handle it if it is) */

	int  (*callback)(int irq_num, void *device);
		/* callback function (to kernel) - when event require
		 * kernel action */

	/* device interface */
	int  (*init)   (uint flags, void *params, device_t *dev);
	int  (*destroy)(uint flags, void *params, device_t *dev);
	int  (*send)   (void *data, size_t size, uint flags, device_t *dev);
	int  (*recv)   (void *data, size_t size, uint flags, device_t *dev);
	int  (*status) (uint flags, device_t *dev);

	/* various flags and parameters specific to device */
	int     flags;
	void   *params;
};
