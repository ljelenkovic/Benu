/*!
 * Formated print on console (using provided interface)
 *
 * Printing from kernel and user programs is same;
 * "just" device can be different
 *
 * "print.c" must define constants: PRINT_NAME, PRINT_ATTRIBUT and DEVICE_SEND
 *
 * For example, kernel/print.c, before including this header must define:
 *
 * #define PRINT_FUNCTION_NAME	kprint
 * #define PRINT_ATTRIBUT	CONSOLE_KERNEL
 * #define DEVICE_SEND(TEXT,SZ)	k_device_send(&TEXT, SZ, 0, k_stdout);
 *
 */
#pragma once

/*
 * macro that uses 'cmd' and 'ind' variables defined in next PRINT_NAME for
 * saving character to buffer for later printing on device
 */
#define	PRINT2BUFFER(CHAR,BUF_SIZE)			\
do {							\
	cmd.cd.print.text[ind++] = (char) (CHAR);	\
	if ( ind == (BUF_SIZE)-1 )			\
	{						\
		cmd.cd.print.text[ind] = 0;		\
		DEVICE_SEND ( cmd, BUF_SIZE );		\
		ind = 0;				\
	}						\
}							\
while (0)

/*! Formated output to console (lightweight version of 'printf') */
int PRINT_FUNCTION_NAME ( char *format, ... )
{
	char **arg = &format;
	int c;
	char buf[20];
	int ind = 0;
	console_cmd_t cmd;

	cmd.cmd = CONSOLE_PRINT;
	cmd.cd.print.attr = PRINT_ATTRIBUT;

	if ( !format )
		return 0;

	arg++; /* first argument after 'format' (on stack) */

	while ( (c = *format++) != 0 )
	{
		if ( c != '%' )
		{
			PRINT2BUFFER ( c, CONSOLE_MAXLEN );
		}
		else {
			char *p;

			c = *format++;
			switch ( c ) {
			case 'd':
			case 'u':
			case 'x':
			case 'X':
				itoa ( buf, c, *((int *) arg++) );
				p = buf;
				while ( *p )
					PRINT2BUFFER ( *p++, CONSOLE_MAXLEN );
				break;

			case 's':
				p = *arg++;
				if ( !p )
					p = "(null)";

				while ( *p )
					PRINT2BUFFER ( *p++, CONSOLE_MAXLEN );
				break;

			default: /* assuming c=='c' */
				PRINT2BUFFER (*((int *) arg++), CONSOLE_MAXLEN);
				break;
			}
		}
	}

	PRINT2BUFFER ( 0, ind+1 );

	return 0;
}

#undef	PRINT2BUFFER
#undef	PRINT_NAME
#undef	PRINT_ATTRIBUT
#undef	DEVICE_SEND
