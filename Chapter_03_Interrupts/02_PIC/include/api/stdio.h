/*! Printing on stdout, reading from stdin */
#pragma once

extern inline int clear_screen ();
extern inline int goto_xy ( int x, int y );
int printf ( char *format, ... );
void warn ( char *format, ... );

int stdio_init ();
