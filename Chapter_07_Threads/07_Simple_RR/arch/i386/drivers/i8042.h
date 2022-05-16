/*! 'keyboard' module */
#ifdef I8042

#pragma once

#include <types/io.h> /* required interface - commands */

/*
Keystroke is represented by single 32-bit number with format:

31------16 15--8 7---0
 x|||||||| ||||| ^^^^^ - ASCII code for key
 x|||||||| ^^^^^ - code for special keys (pressed or released)
  ^^^^^^^^ - special keys that are held while other key is pressed or released

(last bit (31th) is not used)
*/

/* internal code for held special keys (not keyboard codes), for bits 30-16 */
#define LSHIFT		((int32) 1 << 16)
#define RSHIFT		((int32) 1 << 17)
#define LALT		((int32) 1 << 18)
#define RALT		((int32) 1 << 19)
#define LCTRL		((int32) 1 << 20)
#define	RCTRL		((int32) 1 << 21)
#define CAPSL		((int32) 1 << 22)

/* internal code for special keys (as left and right Ctrl, Shift, Alt)
   to recognize when such key is pressed or released; for bits 15-8 */
enum {
	K_LSHIFT_DOWN = 1,
	K_LSHIFT_UP,

	K_RSHIFT_DOWN,
	K_RSHIFT_UP,

	K_LALT_DOWN,
	K_LALT_UP,

	K_RALT_DOWN,
	K_RALT_UP,

	K_LCTRL_DOWN,
	K_LCTRL_UP,

	K_RCTRL_DOWN,
	K_RCTRL_UP,

	K_CAPSL,
	/* etc. */

	KEYS_CNT
};

#define K_SPEC_KEY(KEY)		((KEY) << 8)

/* some flags that might be used on device */
#define ECHO_OFF	1
#define ECHO_ON		2
#define ONLY_LAST	8
#define RAW_KEYSTROKES	16
#define CLEAR_BUFFER	32

#endif /* I8042 */
