/*! Devices - common interface */
#pragma once

#include <types/io.h>

/*! interface to threads */
int sys__open(char *pathname, int flags, mode_t mode, descriptor_t *desc);
int sys__close(descriptor_t *desc);
int sys__read(descriptor_t *desc, void *buffer, size_t count);
int sys__write(descriptor_t *desc, void *buffer, size_t count);
int sys__device_status(descriptor_t *desc, int flags);
int sys__poll(struct pollfd fds[], nfds_t nfds, int timeout,
	descriptor_t *std_desc);
