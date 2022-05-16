/*! Print on console using video memory */

#ifdef VGA_TEXT

#include "../io.h"
#include <arch/device.h>
#include <types/io.h>
#include <lib/string.h>
#include <kernel/errno.h>

#define VIDEO		0x000B8000 /* video memory address */
#define COLS		80 /* number of characters in a column */
#define ROWS		25 /* number of characters in a row */

/*! cursor position */
static int xpos = 0;
static int ypos = 0;

/*! starting address of video memory */
volatile static unsigned char *video = (void *) VIDEO;

/*! font color */
#define COLOR_WHITE	7
#define COLOR_RED	4
#define COLOR_GREEN	2
#define COLOR_DEFAULT	COLOR_WHITE

/*!
 * Supported escape sequences:
 * - "\x1b[2J"		=> clear screen
 * - "\x1b[x;yH"	=> move cursor to y,x (y=row, x=column)
 * - "\x1b[31m"		=> font color = COLOR_RED
 * - "\x1b[32m"		=> font color = COLOR_GREEN
 * - "\x1b[37m"		=> font color = COLOR_WHITE
 * - "\x1b[39m"		=> font color = COLOR_DEFAULT
 */

/*! current font color */
static int font_color = COLOR_DEFAULT;

static int color[3] = {
	7, /* 'normal' characters - white on black background */
	4, /* 'kernel' font - red */
	2  /* 'program' font - green */
};

#define PUT_CHAR(X,Y,CHAR)						\
do {									\
	video [(X + (Y) * COLS) * 2 ] = (CHAR) & 0x00FF;		\
	video [(X + (Y) * COLS) * 2 + 1 ] = font_color;		\
} while (0)


static int vga_text_clear();
static int vga_text_gotoxy(int x, int y);

/*! Init console */
static int vga_text_init(uint flags, void *params, device_t *dev)
{
	video = (unsigned char *) VIDEO;
	xpos = ypos = 0;

	return vga_text_clear();
}

/*! Clear console */
static int vga_text_clear()
{
	int i;

	for (i = 0; i < COLS * ROWS; i++)
	{
		video [2*i] = 0;
		video [2*i+1] = color[2]; /* 'program' style */
	}

	return vga_text_gotoxy(0, 0);
}

/*!
 * Move cursor to specified location
 * \param x Row where to put cursor
 * \param y Column where to put cursor
 */
static int vga_text_gotoxy(int x, int y)
{
	unsigned short int t;

	xpos = x;
	ypos = y;
	t = ypos * 80 + xpos;

	outb(0x3D4, 14);
	outb(0x3D5, t >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, t & 0xFF);

	return 0;
}

/*! Parse escape sequence */
static int vga_process_escape_sequence(char *text)
{
	int i = 0, n, m;

	if (text[i] != '[')
		return i; /* not supported (valid) escape sequence */

	i++;
	if (text[i] >= '0' && text[i] <= '9') {
		n = text[i++] - '0';
		if (text[i] >= '0' && text[i] <= '9')
			n = n * 10 + text[i++] - '0';

		if (text[i] == 'J') {
			if (n == 2)
				vga_text_clear();
			//else => unsupported; ignore

			i++;
		}

		else if (text[i] == 'm') {
			switch(n) {
			case ESC_COLOR_RED:
				font_color = COLOR_RED;
				break;
			case ESC_COLOR_GREEN:
				font_color = COLOR_GREEN;
				break;
			case ESC_COLOR_WHITE:
				font_color = COLOR_WHITE;
				break;
			case ESC_COLOR_DEFAULT:
				font_color = COLOR_DEFAULT;
				break;
			default: break;//ignore
			}

			i++;
		}

		else if (text[i] == ';') {
			i++;
			if (text[i] >= '0' && text[i] <= '9') {
				m = text[i++] - '0';
				if (text[i] >= '0' && text[i] <= '9')
					m = m * 10 + text[i++] - '0';

				if (text[i] == 'H' || text[i] == 'f') {
					if (m <= COLS && n <= ROWS)
						vga_text_gotoxy(m-1, n-1);
					//else => unsupported; ignore
					i++;
				}
				//else => unsupported; ignore
			}
			//else => unsupported; ignore
		}

		//else => unsupported; ignore
	}

	return i;
}

/*! Device wrapper for console
 * Print text string on console, starting at current cursor position
 * \param data String to print
 * \param size size of data
 * \param flags flags for dev
 * \param dev device interface (should be pointer to "vga_text_dev")
 * \return number of characters printed
 */
static int vga_text_send(void *data, size_t size, uint flags, device_t *dev)
{
	char *text = data;
	int i, c, j=0;

	if (!(dev->flags & DEV_TYPE_CONSOLE))
		return EXIT_FAILURE;

	while (text[j])
	{
		if (text[j] == ESCAPE) {
			j++;
			j += vga_process_escape_sequence(&text[j]);
			continue;
		}

		switch(c = text[j++])
		{
		case '\t': /* tabulator */
			xpos = (xpos / 8 + 1) * 8;
			break;

		case '\r': /* carriage return */
			xpos = 0;

		case '\n': /* new line */
			break;

		case '\b': /* backspace */
			if (xpos > 0)
			{
				xpos--;
				PUT_CHAR(xpos, ypos, ' ');
			}
			break;

		default: /* "regular" character */
			PUT_CHAR(xpos, ypos, c);
			xpos++;
		}

		if (xpos >= COLS || c == '\n') /* continue on new line */
		{
			xpos = 0;
			if (ypos < ROWS - 1)
			{
				ypos++;
			}
			else {
				/*scroll one line: move bottom ROWS-1 rows up*/
				for (i = 0; i < COLS * 2 * (ROWS-1); i++)
					video [i] = video [ i + COLS * 2 ];

				for (i = 0; i < COLS; i++)
					PUT_CHAR(i, ROWS-1, ' ');
			}
		}
	}

	vga_text_gotoxy(xpos, ypos);

	return j;
}

/*! Status of device = always ready to print characters on console */
static int vga_text_status(uint flags, device_t *dev)
{
	//ASSERT(dev);

	/* always ready to print characters */
	return DEV_OUT_READY;
}

/*! vga_text as device_t -----------------------------------------------------*/
device_t vga_text_dev = (device_t)
{
	.dev_name =	"VGA_TXT",
	.irq_num = 	-1,
	.irq_handler =	NULL,

	.init =		vga_text_init,
	.destroy =	NULL,
	.send =		vga_text_send,
	.recv =		NULL,
	.status =	vga_text_status,

	.flags = 	DEV_TYPE_SHARED | DEV_TYPE_CONSOLE,
	.params = 	(void *) &vga_text_dev
};


#endif /* VGA_TEXT */
