/*! Print on console using video memory */

#ifdef VGA_TEXT

#include "../io.h"
#include <types/io.h>
#include <lib/string.h>

#define VIDEO		0x000B8000 /* video memory address */
#define COLS		80 /* number of characters in a column */
#define ROWS		25 /* number of characters in a row */

/*! cursor position */
static int xpos = 0;
static int ypos = 0;

/*! starting address of video memory */
volatile static unsigned char *video = (void *) VIDEO;

/*! font color */
static int color[3] = {
	7, /* 'normal' characters - white on black background */
	4, /* 'kernel' font - red */
	2  /* 'program' font - green */
};

#define PUT_CHAR(X,Y,CHAR,ATTR)						\
do {									\
	video [ (X + (Y) * COLS) * 2 ] = (CHAR) & 0x00FF;		\
	if ( (ATTR) == CONSOLE_KERNEL )					\
		video [ (X + (Y) * COLS) * 2 + 1 ] = color[1];		\
	else if ( (ATTR) == CONSOLE_USER )				\
		video [ (X + (Y) * COLS) * 2 + 1 ] = color[2];		\
	else								\
		video [ (X + (Y) * COLS) * 2 + 1 ] = color[0];		\
} while (0)


static int vga_text_clear ();
static int vga_text_gotoxy ( int x, int y );
static int vga_text_printf ( int attr, char *text );

/*! Init console */
static int vga_text_init ( int flags )
{
	video = (unsigned char *) VIDEO;
	xpos = ypos = 0;

	return vga_text_clear ();
}

/*! Clear console */
static int vga_text_clear ()
{
	int i;

	for ( i = 0; i < COLS * ROWS; i++ )
	{
		video [2*i] = 0;
		video [2*i+1] = color[2]; /* 'program' style */
	}

	return vga_text_gotoxy ( 0, 0 );
}

/*!
 * Move cursor to specified location
 * \param x Row where to put cursor
 * \param y Column where to put cursor
 */
static int vga_text_gotoxy ( int x, int y )
{
	unsigned short int t;

	xpos = x;
	ypos = y;
	t = ypos * 80 + xpos;

	outb ( 0x3D4, 14 );
	outb ( 0x3D5, t >> 8 );
	outb ( 0x3D4, 15 );
	outb ( 0x3D5, t & 0xFF );

	return 0;
}

/*!
 * Print text string on console, starting at current cursor position
 * \param data String to print
 */
static int vga_text_printf ( int attr, char *text )
{
	int i, c, j=0;

	while ( text[j] )
	{
		switch ( c = text[j++] )
		{
		case '\t': /* tabulator */
			xpos = ( xpos / 8 + 1 ) * 8;
			break;

		case '\r': /* carriage return */
			xpos = 0;

		case '\n': /* new line */
			break;

		case '\b': /* backspace */
			if ( xpos > 0 )
			{
				xpos--;
				PUT_CHAR ( xpos, ypos, ' ', attr );
			}
			break;

		default: /* "regular" character */
			PUT_CHAR ( xpos, ypos, c, attr );
			xpos++;
		}

		if ( xpos >= COLS || c == '\n' ) /* continue on new line */
		{
			xpos = 0;
			if ( ypos < ROWS - 1 )
			{
				ypos++;
			}
			else {
				/*scroll one line: move bottom ROWS-1 rows up*/
				for ( i = 0; i < COLS * 2 * (ROWS-1); i++ )
					video [i] = video [ i + COLS * 2 ];

				for ( i = 0; i < COLS; i++ )
					PUT_CHAR ( i, ROWS-1, ' ', attr );
			}
		}
	}

	vga_text_gotoxy ( xpos, ypos );

	return strlen ( text );
}

/*! vga_text as console */
console_t vga_text = (console_t)
{
	.init	= vga_text_init,
	.clear	= vga_text_clear,
	.gotoxy	= vga_text_gotoxy,
	.print	= vga_text_printf
};

static int _do_nothing_ ()
{
	return 0;
}

/*! dev_null */
console_t dev_null = (console_t)
{
	.init	= (void *) _do_nothing_,
	.clear	= (void *) _do_nothing_,
	.gotoxy	= (void *) _do_nothing_,
	.print	= (void *) _do_nothing_
};

#endif /* VGA_TEXT */
