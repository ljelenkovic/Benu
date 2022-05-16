/*! Devices - common interface */
#pragma once

#include <kernel/device.h>
#include <arch/device.h>

#ifndef _K_DEVICE_C_

/* "hide" structure that is not required outside this "subsystem" */
typedef void kdevice_t;

#else /* _K_DEVICE_C_ */

#include <lib/list.h>

/*! Kernel device object */
typedef struct _kdevice_t_
{
	device_t   dev;
		   /* device descriptor (with interface) */

	id_t	   id;
		   /* system level id */

	int	   flags;
		   /* is device opened */

	int	   ref_cnt;
		   /* number of processes that opened device */

	list_h	   list;
		   /* all devices are in list */

	list_t	   descriptors;
		   /* list of all descriptor referencing this list */
}
kdevice_t;

#endif /* _K_DEVICE_C_ */

/*! kernel interface */
void kdevice_set_initial_stdout();
int k_devices_init();
kdevice_t *k_device_add(device_t *kdev);
int k_device_init(kdevice_t *kdev, int flags, void *params, void *callback);
int k_device_remove(kdevice_t *kdev);

kdevice_t *k_device_open(char *name, int flags);
void k_device_close(kdevice_t *kdev);

int k_device_send(void *data, size_t size, int flags, kdevice_t *kdev);
int k_device_recv(void *data, size_t size, int flags, kdevice_t *kdev);

int k_device_lock(kdevice_t *dev, int wait);
int k_device_unlock(kdevice_t *dev);
