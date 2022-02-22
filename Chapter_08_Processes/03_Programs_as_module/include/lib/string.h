/*! Memory and string manipulation functions */
#pragma once

#include <types/basic.h>

void *memset	(void *s, int c, size_t n);
void *memsetw	(void *s, int c, size_t n);
void *memcpy	(void *dest, const void *src, size_t n);
void *memmove	(void *dest, const void *src, size_t n);
void *memmovew	(void *dest, const void *src, size_t n);
int   memcmp	(const void *m1, const void *m2, size_t size);

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c);
char *strstr(const char *s1, const char *s2);

char *strtok(char *s, const char *delim);
char *strtok_r(char *s, const char *delim, char **last);

void itoa(char *buffer, int base, int d);
int vssprintf(char *str, size_t size, char **arg);
