/*! Memory and string manipulation functions */

#include <lib/string.h>

/*!
 * Sets the first 'n' bytes of the block of memory pointed by 's' to the
 * specified value 'c' (interpreted as an char)
 *
 * \param s	Pointer to the block of memory to fill
 * \param c	Value to be set (using only lowest 8-bits)
 * \param n	Number of bytes to be set to the value
 * \return s
 */
void *memset(void *s, int c, size_t n)
{
	size_t p;
	char *m = (char *) s;

	for (p = 0; p < n; p++, m++)
		*m = (char) c;

	return s;
}

/*!
 * Sets the first 'n' bytes of the block of memory pointed by 's' to the
 * specified value 'c' (interpreted as an short)
 *
 * \param s	Pointer to the block of memory to fill
 * \param c	Value to be set (using only lowest 16-bits)
 * \param n	Number of bytes to be set to the value
 * \return s
 */
void *memsetw(void *s, int c, size_t n)
{
	size_t p;
	short *m = (short *) s;

	for (p = 0; p < n; p++, m++)
		*m = (short) c;

	return s;
}

/*!
 * Copies the values of 'n' bytes from the location pointed by 'src' directly to
 * the memory block pointed by 'dest'
 *
 * \param dest	Destination address
 * \param src	Source address
 * \param n 	Number of bytes to copy
 * \return dest
 */
void *memcpy(void *dest, const void *src, size_t n)
{
	char *d = (char *) dest, *s = (char *) src;
	size_t p;

	for (p = 0; p < n; p++, d++, s++)
		*d = *s;

	return dest;
}

/*!
 * Copies the values of 'n' bytes from the location pointed by 'src' to the
 * memory block pointed by 'dest'. Copying takes place as if an intermediate
 * buffer was used, allowing the destination and source to overlap.
 *
 * \param dest	Destination address
 * \param src	Source address
 * \param n 	Number of bytes to copy
 * \return dest
 */
void *memmove(void *dest, const void *src, size_t n)
{
	char *d, *s;
	size_t p;

	if (dest < src)
	{
		d = (char *) dest;
		s = (char *) src;
		for (p = 0; p < n; p++, d++, s++)
			*d = *s;
	}
	else {
		d = ((char *) dest) + n - 1;
		s = ((char *) src) + n - 1;
		for (p = 0; p < n; p++, d--, s--)
			*d = *s;
	}

	return dest;
}

/*!
 * Copies the values of 'n' 16-bit words from the location pointed by 'src' to
 * the memory block pointed by 'dest'. Copying takes place as if an intermediate
 * buffer was used, allowing the destination and source to overlap.
 *
 * \param dest	Destination address
 * \param src	Source address
 * \param n 	Number of words (16-bit) to copy
 * \return dest
 */
void *memmovew(void *dest, const void *src, size_t n)
{
	short int *d, *s;
	size_t p;

	if (dest < src)
	{
		d = (short int *) dest;
		s = (short int *) src;
		for (p = 0; p < n; p++, d++, s++)
			*d = *s;
	}
	else {
		d = ((short int *) dest) + n - 1;
		s = ((short int *) src) + n - 1;
		for (p = 0; p < n; p++, d--, s--)
			*d = *s;
	}

	return dest;
}

/*!
 * Compares the first 'size' bytes of the block of memory pointed by 'm1' to the
 * first 'size' bytes pointed by 'm2'
 * \param m1	First block
 * \param m2	Second block
 * \param size	Number of bytes to compare
 * \return Returns an integral value indicating the relationship between the
 *	   content of the memory blocks:
 *		A zero value indicates that the contents of both memory blocks
 *		are equal.
 *		A value greater than zero indicates that the first byte that
 *		does not match in both memory blocks has a greater value in 'm1'
 *		than in 'm2' as if evaluated as unsigned char values; And a
 *		value less than zero indicates the opposite.
 */
int memcmp(const void *m1, const void *m2, size_t size)
{
	unsigned char *a = (unsigned char *) m1;
	unsigned char *b = (unsigned char *) m2;

	for (; size > 0; a++, b++, size--)
	{
		if (*a < *b)
			return -1;
		else if (*a > *b)
			return 1;
	}

	return 0;
}

/*! Returns string length */
size_t strlen(const char *s)
{
	size_t i;

	for (i = 0; s[i]; i++)
		;

	return i;
}

/*! Returns -1 if s1 < s2, 0 if s1 == s2 or 1 if s1 > s2 */
int strcmp(const char *s1, const char *s2)
{
	size_t i;

	for (i = 0; s1[i] || s2[i]; i++)
	{
		if (s1[i] < s2[i])
			return -1;
		else if (s1[i] > s2[i])
			return 1;
	}
	return 0;
}

/*! Returns -1 if s1 < s2, 0 if s1 == s2 or 1 if s1 > s2 */
int strncmp(const char *s1, const char *s2, size_t n)
{
	size_t i;

	for (i = 0; i < n && (s1[i] || s2[i]); i++)
	{
		if (s1[i] < s2[i])
			return -1;
		else if (s1[i] > s2[i])
			return 1;
	}
	return 0;
}

/*!
 * Copy string
 * \param dest Destination string
 * \param src Source string
 * \return dest
 */
char *strcpy(char *dest, const char *src)
{
	int i;

	for (i = 0; src[i]; i++)
		dest[i] = src[i];

	dest[i] = 0;

	return dest;
}

/*!
 * Append one string to another
 * \param dest String to append
 * \param src String which will be copied
 * \return dest
 */
char *strcat(char *dest, const char *src)
{
	int i;

	for (i = 0; dest[i]; i++)
		;

	strcpy(&dest[i], src);

	return dest;
}

/*!
 * Search for character in string.
 * \param s String
 * \param c Character
 * \return Pointer to first appearance of c in s, NULL if none
 */
char *strchr(const char *s, int c)
{
	int i;

	for (i = 0; s[i]; i++)
		if (s[i] == (char) c)
			return (char *) &s[i];
	return NULL;
}

/*!
 * Search a substring (first occurrence).
 * \param s1 String to be searched
 * \param s2 String that is searched in s1
 * \return Pointer to first appearance of s2 in s1, NULL if none
 */
char *strstr(const char *s1, const char *s2)
{
	int j;

	for (; s1 && s2 ;)
	{
		for (j = 0; s1[j] && s2[j] && s1[j] == s2[j]; j++)
			;

		if (!s2[j])
			return (char *) s1;

		if (!s1[j]) /* s1 is shorter than s2 */
			return NULL;

		s1++;
	}

	return (char *) s1;
}

/*!
 * Convert integer number to string (ASCII)
 * \param buf	Address where to store string representation of given number
 * \param base	Number base ('d', 'u', 'x' or 'X')
 * \param d	Number to convert
 */
void itoa(char *buffer, int base, int d)
{
	char *p = buffer;
	char *p1, *p2, firsthexchar;
	unsigned long ud = d;
	int divisor = 10;
	int digits = 0;

	/* if number is negative and format is '%d', insert starting '-' */
	if (base == 'd' && d < 0)
	{
		*p++ = '-';
		 buffer++;
		ud = -d;
	}
	else if (base == 'x' || base == 'X')
	{
		divisor = 16;
	}

	firsthexchar = (base == 'x' ? 'a' : 'A');

	/* divide 'ud' with base 'divisor' until 'ud' is not 0 */
	do {
		int remainder = ud % divisor;

		*p++ = (remainder < 10) ? remainder + '0' :
					remainder + firsthexchar - 10;
		digits++;
	}
	while (ud /= divisor);

	/* add leading zeros if hexadecimal format is required */
	if (base == 'x' || base == 'X')
	{
		while (digits < 8)
		{
			digits++;
			*p++ = '0';
		}
		*p++ = 'x';
		*p++ = '0';
	}
	/* Add terminating character */
	*p = 0;

	/* Reverse string */
	p1 = buffer;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}


/*! Formated output to console (lightweight version of 'printf') */
int vssprintf(char *str, size_t size, char **arg)
{
	char *format = *arg, buffer[20], *p;
	int c, i = 0;

	if (!format)
		return 0;

	arg++; /* first argument after 'format' (on stack) */

	while ((c = *format++) != 0 && i < size - 1)
	{
		if (c != '%')
		{
			str[i++] = (char) c;
		}
		else {
			c = *format++;
			switch(c) {
			case 'd':
			case 'u':
			case 'x':
			case 'X':
				itoa(buffer, c, *((int *) arg++));
				p = buffer;
				if (i + strlen(p) < size - 1)
					while (*p)
						str[i++] = *p++;
				else
					goto too_long;
				break;

			case 's':
				p = *arg++;
				if (!p)
					p = "(null)";

				if (i + strlen(p) < size - 1)
					while (*p)
						str[i++] = *p++;
				else
					goto too_long;
				break;

			default: /* assuming c=='c' */
				str[i++] = *((int *) arg++);
				break;
			}
		}
	}

too_long: /* just print what did fit */

	str[i++] = 0;

	return i;
}


/*! strtok and strtok_r are reused from: */

/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

char *strtok(char *s, const char *delim)
{
	static char *last;

	return strtok_r(s, delim, &last);
}

char *strtok_r(char *s, const char *delim, char **last)
{
	int i, j, found_delim;

	if (s == NULL)
		s = *last;
	if (s == NULL)
		return NULL;

	/* skip leading delimiters */
	for (i = 0; s[i] != 0; i++)
	{
		found_delim = 0;
		for (j = 0; delim[j] != 0; j++)
		{
			if (s[i] == delim[j])
			{
				found_delim = 1;
				break;
			}
		}

		if (found_delim == 0)
			break;
	}

	if (s[i] == 0)
	{
		*last = NULL;
		return NULL;
	}

	s = &s[i];

	/* search for next delimiter */
	for (i = 1; s[i] != 0; i++)
	{
		found_delim = 0;
		for (j = 0; delim[j] != 0; j++)
		{
			if (s[i] == delim[j])
			{
				found_delim = 1;
				break;
			}
		}

		if (found_delim == 1)
			break;
	}

	if (s[i] == 0)
		*last = NULL;
	else
		*last = &s[i+1];
	s[i] = 0;

	return s;
}

#if 0 /* optimized (original) version */
char *strtok_r(char *s, const char *delim, char **last)
{
	char *spanp;
	int c, sc;
	char *tok;


	if (s == NULL && (s = *last) == NULL)
		return NULL;

	/*
	 * Skip (span) leading delimiters(s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (char *) delim;(sc = *spanp++) != 0;)
		if (c == sc)
			goto cont;

	if (c == 0)	/* no non-delimiter characters */
	{
		*last = NULL;
		return NULL;
	}

	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;)
	{
		c = *s++;
		spanp = (char *) delim;
		do
		{
			if ((sc = *spanp++) == c)
			{
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*last = s;
				return tok;
			}
		}
		while (sc != 0);
	}
	/* NOTREACHED */
}
#endif
