/*! Printing on stdout, reading from stdin */
#pragma once

#include <types/io.h>

int open(char *pathname, int flags, mode_t mode);
int close(int fd);
ssize_t read(int fd, void *buffer, size_t count);
ssize_t write(int fd, void *buffer, size_t count);

int getchar();
int printf(char *format, ...);
void warn(char *format, ...);

int stdio_init();

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
