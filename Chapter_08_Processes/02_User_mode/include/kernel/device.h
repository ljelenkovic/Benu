/*! Devices - common interface */
#pragma once

/*! interface to threads (via syscall) */
int sys__open(void *p);
int sys__close(void *p);
int sys__read(void *p);
int sys__write(void *p);
int sys__device_status(void *p);
int sys__poll(void *p);
