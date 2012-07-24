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
void *memset ( void *s, int c, size_t n )
{
	size_t p;
	char *m = (char *) s;

	for ( p = 0; p < n; p++, m++ )
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
void *memsetw (void *s, int c, size_t n)
{
	size_t p;
	short *m = (short *) s;

	for ( p = 0; p < n; p++, m++ )
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
void *memcpy ( void *dest, const void *src, size_t n )
{
	char *d = (char *) dest, *s = (char *) src;
	size_t p;

	for ( p = 0; p < n; p++, d++, s++ )
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
void *memmove ( void *dest, const void *src, size_t n )
{
	char *d, *s;
	size_t p;

	if ( dest < src )
	{
		d = (char *) dest;
		s = (char *) src;
		for ( p = 0; p < n; p++, d++, s++ )
			*d = *s;
	}
	else {
		d = ((char *) dest) + n - 1;
		s = ((char *) src) + n - 1;
		for ( p = 0; p < n; p++, d--, s-- )
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
void *memmovew ( void *dest, const void *src, size_t n )
{
	short int *d, *s;
	size_t p;

	if ( dest < src )
	{
		d = (short int *) dest;
		s = (short int *) src;
		for ( p = 0; p < n; p++, d++, s++ )
			*d = *s;
	}
	else {
		d = ((short int *) dest) + n - 1;
		s = ((short int *) src) + n - 1;
		for ( p = 0; p < n; p++, d--, s-- )
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
int memcmp ( const void *m1, const void *m2, size_t size )
{
	unsigned char *a = (unsigned char *) m1;
	unsigned char *b = (unsigned char *) m2;

	for ( ; size > 0; a++, b++, size-- )
	{
		if ( *a < *b )
			return -1;
		else if ( *a > *b )
			return 1;
	}

	return 0;
}

/*! Returns string length */
size_t strlen ( const char *s )
{
	size_t i;

	for ( i = 0; s[i]; i++ )
		;

	return i;
}

/*! Returns -1 if s1 < s2, 0 if s1 == s2 or 1 if s1 > s2 */
int strcmp ( const char *s1, const char *s2 )
{
	size_t i;

	for ( i = 0; s1[i] || s2[i]; i++ )
	{
		if ( s1[i] < s2[i] )
			return -1;
		else if ( s1[i] > s2[i] )
			return 1;
	}
	return 0;
}

/*! Returns -1 if s1 < s2, 0 if s1 == s2 or 1 if s1 > s2 */
int strncmp ( const char *s1, const char *s2, size_t n )
{
	size_t i;

	for ( i = 0; i < n && ( s1[i] || s2[i] ); i++ )
	{
		if ( s1[i] < s2[i] )
			return -1;
		else if ( s1[i] > s2[i] )
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
char *strcpy ( char *dest, const char *src )
{
	int i;

	for ( i = 0; src[i]; i++ )
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
char *strcat ( char *dest, const char *src )
{
	int i;

	for ( i = 0; dest[i]; i++ )
		;

	strcpy ( &dest[i], src );

	return dest;
}

/*!
 * Search for character in string.
 * \param s String
 * \param c Character
 * \return Pointer to first appearance of c in s, NULL if none
 */
char *strchr (const char *s, int c)
{
	int i;

	for ( i = 0; s[i]; i++ )
		if ( s[i] == (char) c )
			return (char *) &s[i];
	return NULL;
}

/*!
 * Search a substring (first occurrence).
 * \param s1 String to be searched
 * \param s2 String that is searched in s1
 * \return Pointer to first appearance of s2 in s1, NULL if none
 */
char *strstr (const char *s1, const char *s2)
{
	int j;

	for (; s1 && s2 ;)
	{
		for ( j = 0; s1[j] && s2[j] && s1[j] == s2[j]; j++ )
			;

		if ( !s2[j] )
			return (char *) s1;

		if ( !s1[j] ) /* s1 is shorter than s2 */
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
void itoa ( char *buf, int base, int d )
{
	char *p = buf;
	char *p1, *p2, firsthexchar;
	unsigned long ud = d;
	int divisor = 10;
	int digits = 0;

	/* if number is negative and format is '%d', insert starting '-' */
	if ( base == 'd' && d < 0 )
	{
		*p++ = '-';
		buf++;
		ud = -d;
	}
	else if ( base == 'x' || base == 'X' )
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
	while ( ud /= divisor );

	/* add leading zeros if hexadecimal format is required */
	if ( base == 'x' || base == 'X' )
	{
		while ( digits < 8 )
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
	p1 = buf;
	p2 = p - 1;
	while ( p1 < p2 )
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}


/*! Formated output to console (lightweight version of 'printf') */
int vssprintf ( char *str, size_t size, char **arg )
{
	char *format = *arg, buf[20], *p;
	int c, i = 0;

	if ( !format )
		return 0;

	arg++; /* first argument after 'format' (on stack) */

	while ( (c = *format++) != 0 && i < size - 1 )
	{
		if ( c != '%' )
		{
			str[i++] = (char) c;
		}
		else {
			c = *format++;
			switch ( c ) {
			case 'd':
			case 'u':
			case 'x':
			case 'X':
				itoa ( buf, c, *((int *) arg++) );
				p = buf;
				if ( i + strlen (p) < size - 1 )
					while ( *p )
						str[i++] = *p++;
				else
					goto too_long;
				break;

			case 's':
				p = *arg++;
				if ( !p )
					p = "(null)";

				if ( i + strlen (p) < size - 1 )
					while ( *p )
						str[i++] = *p++;
				else
					goto too_long;
				break;

			default: /* assuming c=='c' */
				str[i++] = *( (int *) arg++ );
				break;
			}
		}
	}

too_long: /* just print what did fit */

	str[i++] = 0;

	return i;
}