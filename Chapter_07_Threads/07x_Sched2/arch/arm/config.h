/*! arm system configuration */

#pragma once

enum {
	PB926EJS = 1, /* implemented */
	RASPBERRYPI, /* not (yet) implemented */
};

/* choose platform */
#define ARM_SYSTEM	PB926EJS

#if (ARM_SYSTEM == PB926EJS)

/*! Platform: Versatile PB926EJ-S tested with QEMU */

#define UART0_BASE	0x101f1000	/* UART0 base address */
#define VICBASE		0x10140000	/* VIC base address */
#define SICBASE		0x10003000	/* SIC base address */
#define TIMER0_BASE	0x101E2000
#define TIMER1_BASE	0x101E2020


/* #elseif ... */

#endif
