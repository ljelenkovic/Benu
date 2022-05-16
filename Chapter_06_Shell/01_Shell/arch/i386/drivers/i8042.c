/*! 'keyboard' module (with i8042 chip) */
#ifdef I8042

#include "i8042.h"

#include "../io.h"
#include "../interrupt.h"
#include "../processor.h"
#include <kernel/errno.h>
#include <arch/device.h>

#define KEYB_DR		0x60	/* data register */
#define KEYB_SR		0x64	/* status register */

#define KEYB_BUFF_SIZE	256	/* keyboard buffer size (software buffer) */

#define CAPSLED		0x04	/* code for turning on Caps Lock LED */

#define ASCII_KEY(KEY)	((KEY) & 0x000000ff)	/* leave only ASCII */

#define SHIFT_ON(S)	((((S) &(LSHIFT | RSHIFT)) && !((S) & CAPSL)) || \
			 (!((S) &(LSHIFT | RSHIFT)) && ((S) & CAPSL))   )

/* internal functions */
static int i8042_init(uint flags, void *kernel_callback_function, device_t *d);
static int i8042_destroy(uint flags, void *params, device_t *d);
static int i8042_send(void *data, size_t size, uint flags, device_t *d);
static int i8042_get(void *data, size_t size, uint flags, device_t *d);
static int i8042_interrupt_handler(int irq_num, void *device);

static void i8042_send_cmd(int cmd);
static void i8042_set_leds();
static int32 i8042_prepare_char(int c);
static int32 i8042_read();
static int i8042_insert_keystroke(int c);


/*! circular buffer for last few keystrokes */
static volatile int32 keyb_buffer[KEYB_BUFF_SIZE];
static volatile int buf_first;
static volatile int buf_last;
static volatile int buf_size;

/*! which special keys (Shift, Ctrl, Alt) are pressed */
static volatile int32 spec_keys_down;

static volatile uint32 keyb_flags; /*! echo any key pressed? */

static void (*kernel_interrupt_callback_function)(void) = NULL;

/*! init keyboard 'module' */
static int i8042_init(uint flags, void *kernel_callback_function, device_t *d)
{
	buf_first = buf_last = buf_size = 0;
	spec_keys_down = (int32) 0;
	keyb_flags = flags;

	/* empty hardware buffer */
	while (i8042_read() > 0)
		;

	kernel_interrupt_callback_function = kernel_callback_function;

	return 0;
}

/*! disable keyboard module */
static int i8042_destroy(uint flags, void *params, device_t *d)
{
	kernel_interrupt_callback_function = NULL;

	return 0;
}

/*! "push back" char in buffer */
static int i8042_send(void *data, size_t size, uint flags, device_t *d)
{
	int i, *chars = (int *) data;

	for (i = 0; i < size; i++)
		i8042_insert_keystroke(chars[i]);

	return 0;
}

/*! keyboard module is character device - in this implementation return only
   single number that represent keystroke */
static int i8042_get(void *data, size_t size, uint flags, device_t *d)
{
	int32 key = (int32) 0;

	if (buf_size > 0)
	{
		if ((flags & ONLY_LAST))
		{
			/* return last keystroke, drop all before */
			buf_first = (buf_last>0 ? buf_last-1 : KEYB_BUFF_SIZE);
			buf_size = 1;
		}

		do {
			key = keyb_buffer[buf_first];
			buf_first = (buf_first + 1) % KEYB_BUFF_SIZE;
			buf_size--;
		}
		while ((flags & CONSOLE_ASCII) && !(key = ASCII_KEY(key))
			&& buf_size > 0);

		if (flags & CLEAR_BUFFER)
			buf_first = buf_last = buf_size = 0;

		if (key && data)
			*((int32 *) data) = key;
	}

	return (key ? 1 : 0);
}

/*! Keyboard interrupt handler - read new keystrokes and process them */
static int i8042_interrupt_handler(int irq_num, void *device)
{
	int32 c;
	int new_keystrokes = FALSE;

	while ((c = i8042_read()) >= 0)
	{
		new_keystrokes |= i8042_insert_keystroke(c);

		if (ASCII_KEY(c) && (keyb_flags & ECHO_ON))
			kprintf("%c", ASCII_KEY(c));
	}

	/* Example: halt on Ctrl+p
	 * if (ASCII_KEY(c) == 'p' && (spec_keys_down & LCTRL))
	 *	halt();
	 */

	if (new_keystrokes && kernel_interrupt_callback_function)
		kernel_interrupt_callback_function();

	return new_keystrokes;
}

/*! Insert keystroke into keyboard software buffer */
static int i8042_insert_keystroke(int c)
{
	if (c == (int32) 0) /* empty keystroke changes nothing */
		return 0;

	if ((keyb_flags & CONSOLE_ASCII) && ASCII_KEY(c) == 0)
		return 0;

	/* if buffer is full - overwrite oldest keystroke */

	keyb_buffer[buf_last] = c | spec_keys_down;

	buf_last = (buf_last + 1) % KEYB_BUFF_SIZE;

	if (buf_size < KEYB_BUFF_SIZE)
		buf_size++; /* buffer not full */
	else
		buf_first = (buf_first + 1) % KEYB_BUFF_SIZE;
		/* buffer full, oldest keystroke is overwritten with new */

	return 1;
}

/*! translation table from keyboard scan code to ASCII */
static char keyb_codes[128] =
{
  0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0','\'', '+','\b','\t',
'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', '{', '}','\n',   0, 'a', 's',
'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`',   0,'\\', 'y', 'x', 'c', 'v',
'b', 'n', 'm', ',', '.', '-',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
'2', '3', '0', '.'
};

/*! translation table from keyboard scan code to ASCII with shift/CAPSLOCK on */
static char keyb_codes_shift[128] =
{
  0,   0, '!', '"', '#', '$', '%', '&', '/', '(', ')', '=',   0, '*','\b','\t',
'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '[', ']','\n',   0, 'A', 'S',
'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '"',   0,   0, '|', 'Y', 'X', 'C', 'V',
'B', 'N', 'M', ';', ':', '_',   0,   0,   0, ' ',   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
'2', '3', '0', '.'
};

/*!
 * Read keystroke from keyboard controller
 * \return keystroke code or -1 if no answer from controller
 */
static int32 i8042_read()
{
	if (!(inb(KEYB_SR) & 0x01)) /* has controller new data? */
		return EXIT_FAILURE;

	if (keyb_flags & RAW_KEYSTROKES)
		return (int32) inb(KEYB_DR);
	else
		return i8042_prepare_char(inb(KEYB_DR));
}

/*!
 * Translate keyboard code into more appropriate code (defined in keyboard.h)
 * process only (some) simple keystrokes and (some) two byte keystroke codes
 */
static int32 i8042_prepare_char(int c)
{
	static int multi_code = 0;
	int32 key = (int32) 0;

	if (c == 0xE0 && !multi_code)
	{
		multi_code = 1;
		return key;
	}

	if (!multi_code)
	{
		switch(c) {
		case 0xBA: /* CAPS LOCK */
			spec_keys_down ^= CAPSL;
			key = K_SPEC_KEY(K_CAPSL);
			i8042_set_leds();
			break;

		case 0xAA: /* left SHIFT released */
			spec_keys_down &= ~ LSHIFT;
			key = K_SPEC_KEY(K_LSHIFT_UP);
			break;

		case 0x2A: /* left SHIFT down */
			spec_keys_down |= LSHIFT;
			key = K_SPEC_KEY(K_LSHIFT_DOWN);
			break;

		case 0xB6: /* right SHIFT released */
			spec_keys_down &= ~ RSHIFT;
			key = K_SPEC_KEY(K_RSHIFT_UP);
			break;

		case 0x36: /* right SHIFT down */
			spec_keys_down |= RSHIFT;
			key = K_SPEC_KEY(K_RSHIFT_DOWN);
			break;

		case 0x1D: /* left CTRL down */
			spec_keys_down |= LCTRL;
			key = K_SPEC_KEY(K_LCTRL_DOWN);
			break;

		case 0x9D: /* left CTRL up */
			spec_keys_down &= ~ LCTRL;
			key = K_SPEC_KEY(K_LCTRL_UP);
			break;

		case 0x38: /* left ALT down */
			spec_keys_down |= LALT;
			key = K_SPEC_KEY(K_LALT_DOWN);
			break;

		case 0xB8: /* left ALT up */
			spec_keys_down &= ~ LALT;
			key = K_SPEC_KEY(K_LALT_UP);
			break;

		default:	/* maybe 'normal' char ? */
			if (c < 128)
			{
				if (SHIFT_ON(spec_keys_down))
					key = (int32) keyb_codes_shift[c];
				else
					key = (int32) keyb_codes[c];
			}
			break;
		}
	}
	else {
		/* multi byte code, E0 was previous code */

		switch(c) {
		case 0x1D: /* right CTRL down */
			spec_keys_down |= RCTRL;
			key = K_SPEC_KEY(K_RCTRL_DOWN);
			break;

		case 0x9D: /* right CTRL up */
			spec_keys_down &= ~ RCTRL;
			key = K_RCTRL_UP;
			break;

		case 0x38: /* right ALT down */
			spec_keys_down |= RALT;
			key = K_SPEC_KEY(K_RALT_DOWN);
			break;

		case 0xB8: /* right ALT up */
			spec_keys_down &= ~ RALT;
			key = K_SPEC_KEY(K_RALT_UP);
			break;
		}

		multi_code = 0;
	}

	return key;
}

/*! Send command to keyboard controller */
static void i8042_send_cmd(int cmd)
{
	/* busy wait until controller is ready */
	while (inb(KEYB_SR) & 0x02)
		;

	/* send command */
	outb(KEYB_DR, (unsigned char) cmd);
}

/*! turn on/off CAPS LOCK led */
static void i8042_set_leds()
{
	i8042_send_cmd(0xED);

	if (spec_keys_down & CAPSL)
		i8042_send_cmd(CAPSLED);
	else
		i8042_send_cmd(0);
}

/*! Get status */
static int i8042_status(uint flags, device_t *dev)
{
	int rflags = 0;

	/* look up software buffers */
	if (buf_size > 0)
		rflags |= DEV_IN_READY;

	if (buf_size < KEYB_BUFF_SIZE)
		rflags |= DEV_OUT_READY;

	return rflags;
}

/*! device_t interface  ------------------------------------------------------*/
device_t i8042_dev = (device_t)
{
	.dev_name = "i8042",

	.irq_num = 	IRQ_KEYBOARD,
	.irq_handler =	i8042_interrupt_handler,

	.init =		i8042_init,
	.destroy =	i8042_destroy,
	.send =		i8042_send,
	.recv =		i8042_get,
	.status =	i8042_status,

	.flags = 	DEV_TYPE_SHARED,
	.params = 	NULL,
};

#endif /* I8042 */
