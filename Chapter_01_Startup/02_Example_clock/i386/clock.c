/*! Emulate clock (at the center of the screen) */

/* data types and functions prototypes */
typedef	unsigned char 		uint8;
typedef	unsigned short int	uint16;
typedef	unsigned int 		uint;

static inline void outb(uint16 port, uint8 data);
static inline uint8 inb(uint16 port);
static void i8253_set(uint cnt);
static uint i8253_get();
static void init_display();
static inline void update_field(int pos, int num);

/*! video memory */
#define VIDEO	((volatile char *) 0x000B8000) /* video memory address */
#define COLS	80 /* number of characters in a column */
#define ROWS	25 /* number of characters in a row */
#define ATTR	2  /* font: green char on black bacground */

/*! i8253 ports and commands */
#define	I8253_CH0	0x40
#define	I8253_CMD	0x43
#define	I8253_CMD_LOAD	0x34
#define	I8253_CMD_LATCH	0x04

#define	I8253_FREQ	1193181 /* counter frequency */
#define COUNT_10MS	(I8253_FREQ / 100)

/*! Clock format: hh:mm:ss:hs, positions of each element: */
#define HOURS	0
#define MINS	3
#define SECS	6
#define HUND	9

#define CLOCK_POS	(VIDEO + ((ROWS / 2) * COLS + COLS / 2 - 6) * 2)

/*! Emulate clock (at the center of the screen) */
void clock()
{
	int hours, mins, secs, hund;

	init_display();
	/* initial clock */
	hours = mins = secs = hund = 0;

	update_field(HOURS, hours);
	update_field(MINS, mins);
	update_field(SECS, secs);
	update_field(HUND, hund);

	/* reset counter */
	i8253_set(COUNT_10MS);

	while (1)
	{
		while (i8253_get() != COUNT_10MS) /* busy waiting */
			; /* asm volatile ("":::"memory"); */

		hund++;

		if (hund > 99)
		{
			hund = 0;

			secs++;

			if (secs > 59)
			{
				secs = 0;

				mins++;

				if (mins > 59)
				{
					mins = 0;

					hours++;

					if (hours > 23)
						hours = 0;

					update_field(HOURS, hours);
				}

				update_field(MINS, mins);
			}

			update_field(SECS, secs);
		}

		update_field(HUND, hund);

		while (i8253_get() == COUNT_10MS) /* busy waiting */
			; /* asm volatile ("":::"memory"); */
	}
}

/*! Set 2 digit number on console at requested position */
static void init_display()
{
	volatile char *video = VIDEO;
	int i;

	/* erase screen (set blank screen) */
	for (i = 0; i < COLS * ROWS; i++)
	{
		video[i*2] = 0;
		video[i*2+1] = ATTR;
	}

	video = CLOCK_POS;
	video [ 2 * 2 ] = ':';
	video [ 5 * 2 ] = ':';
	video [ 8 * 2 ] = ':';
}

/*! Set 2 digit number on console at requested position */
static inline void update_field(int pos, int num)
{
	volatile char *video = CLOCK_POS;
	char c1, c0;

	c1 = num / 10 + '0';
	c0 = num % 10 + '0';

	pos *= 2;
	video [ pos++ ] = c1;
	pos++;
	video [ pos++ ] = c0;
	pos++;
}


/*! Write to 8-bit port */
static inline void outb(uint16 port, uint8 data)
{
	asm ("outb %b0, %w1" : : "a" (data), "d" (port));
}

/*! Read from 8-bit port */
static inline uint8 inb(uint16 port)
{
	uint8 r;

	asm volatile ("inb %w1, %b0" : "=a" (r) : "d" (port));

	return r;
}

/*! Load i8253 counter with 'cnt' */
static void i8253_set(uint cnt)
{
	uint8 counter;

	outb(I8253_CMD, I8253_CMD_LOAD);

	counter = (uint8)(cnt & 0x00ff);
	outb(I8253_CH0, counter);

	counter = (uint8)((cnt >> 8) & 0x00ff);
	outb(I8253_CH0, counter);
}

/*! Read i8253 counter (its current value) */
static uint i8253_get()
{
	uint lower_byte, higher_byte;

	outb(I8253_CMD, I8253_CMD_LATCH);
	lower_byte = inb(I8253_CH0);
	higher_byte = inb(I8253_CH0);

	return lower_byte + (higher_byte << 8);
}
