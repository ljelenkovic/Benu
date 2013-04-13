/*! Printing on stdout, reading from stdin */
#pragma once

#include <types/io.h>

int open ( char *pathname, int flags, mode_t mode );
int close ( int fd );
ssize_t read ( int fd, void *buffer, size_t count );
ssize_t write ( int fd, void *buffer, size_t count );

extern inline int get_char ();
extern inline int clear_screen ();
extern inline int goto_xy ( int x, int y );
int printf ( char *format, ... );
void warn ( char *format, ... );

int stdio_init ();
