/*! PrimeCell Vectored Interrupt Controller (VIC, PL190) */
#ifdef PL190

#pragma once

#ifndef ASM_FILE

#include <ARCH/config.h>

/*
 * Tecnical data from:
 * RealView Platform Baseboard for ARM926EJ-S™, HBI-0117, User Guide
 */

/*! Primary interrupt Controller (VIC, PL190) --------------------------------- */

/*! VIC control registers */

/*      Register		Offset   Type Reset value, Description */
#define VICIRQSTATUS	0x000 /* RO   0x00000000   IRQ Status Reg. */
#define VICFIQSTATUS	0x004 /* RO   0x00000000   FIQ Status Reg. */
#define VICRAWINTR	0x008 /* RO   -            Raw Int. Status Reg. */
#define VICINTSELECT	0x00C /* R/W  0x00000000   Int. Select Reg. */
#define VICINTENABLE	0x010 /* R/W  0x00000000   Int. Enable Reg. */
#define VICINTENCLEAR	0x014 /* W    -            Int. Enable Clear Reg. */
#define VICSOFTINT	0x018 /* R/W  0x00000000   Software Int. Reg. */
#define VICSOFTINTCLEAR	0x01C /* WO   -            Software Int. Clear Reg. */
#define VICPROTECTION	0x020 /* R/W  0x0          Protection Enable Reg. */

#if 0	/*! Not using vectored interrupts !*/
#define VICVECTADDR	0x030 /* R/W  0x00000000   Vector Address Reg. */
#define VICDEFVECTADDR	0x034 /* R/W  0x00000000   Default Vector Address Reg. */

#define VICVECTADDR0	0x100 /* R/W	0x00000000 Vector Address Reg.s */
/* ... */
#define VICVECTADDR15	0x13c
#define VICVECTCNTL0	0x200 /* R/W	0x00000000 Vector Control Reg.s */
/* ... */
#define VICVECTCNTL15	0x23c /* R/W	0x00000000 Vector Control Reg.s */
#endif


/*! Interrupts generated through VIC (connected to VIC) */
enum {
/*0*/	WATCHDOG = 0,	/* Watchdog timer */
/*1*/	SWI,		/* Software interrupt (not one generated with SVC/SWI) */
/*2*/	COM_RX,		/* Debug communications receive interrupt */
/*3*/	COM_TX,		/* Debug communications transmit interrupt */
/*4*/	TIMER01,	/* Timer 0 or 1 Timers on development chip */
/*5*/	TIMER23,	/* Timer 2 or 3 Timers on development chip */
/*6*/	GPIO0,		/* GPIO controller in development chip */
/*7*/	GPIO1,		/* GPIO controller in development chip */
/*8*/	GPIO2,		/* GPIO controller in development chip */
/*9*/	GPIO3,		/* GPIO controller in development chip */
/*10*/	RTC,		/* Real time clock in development chip */
/*11*/	SSP,		/* Synchronous serial port in development chip */
/*12*/	UART0IRQL,	/* UART0 on development chip */
/*13*/	UART1IRQL,	/* UART1 on development chip */
/*14*/	UART2IRQL,	/* UART2 on development chip */
/*15*/	SCI0,		/* Smart Card interface in development chip */
/*16*/	CLCD,		/* CLCD controller in development chip */
/*17*/	DMA,		/* DMA controller in development chip */
/*18*/	PWRFAIL,	/* Power failure from FPGA */
/*19*/	MBX,		/* Graphics processor on development chip */
/*20*/	GND,		/* Reserved */
/*21*/	VICINTSOURCE21, /* External int. signal from RealView Logic Tile or .. */
/*22*/	VICINTSOURCE22,
/*23*/	VICINTSOURCE23,
/*24*/	VICINTSOURCE24,
/*25*/	VICINTSOURCE25,
/*26*/	VICINTSOURCE26,
/*27*/	VICINTSOURCE27,
/*28*/	VICINTSOURCE28,
/*29*/	VICINTSOURCE29,
/*30*/	VICINTSOURCE30,
/*31*/	VICINTSOURCE31, /* External interrupt from secondary controller */
};


/*! Secondary interrupt controller (SIC) -------------------------------------- */
/* A secondary interrupt controller is implemented as a custom design in the
 * FPGA and connected to 31 pin of primary controller. */

/* 	Name 		Address	  Access	 Description */
#define SIC_STATUS	0x0000 /* R	Status of interrupt (after mask) */
#define SIC_RAWSTAT	0x0004 /* R	Status of interrupt (before mask) */
#define SIC_ENABLE	0x0008 /* R	Interrupt mask */
#define SIC_ENSET	0x0008 /* W	Set bits HIGH to enable the corresponding interrupt signals */
#define SIC_ENCLR	0x000C /* W	Set bits HIGH to mask the corresponding interrupt signals */
#define SIC_SOFTINTSET	0x0010 /* R/W	Set software interrupt */
#define SIC_SOFTINTCLR	0x0014 /* W	Clear software interrupt */
#define SIC_PICENABLE	0x0020 /* R	Read status of pass-through mask (allows interrupt to pass directly to the primary interrupt controller) */
#define SIC_PICENSET	0x0020 /* W	Set bits HIGH to set the corresponding interrupt pass-through mask bits */
#define SIC_PICENCLR	0x0024 /* W	Set bits HIGH to clear the corresponding interrupt pass-through mask bits */

/*! Interrupts generated through SIC (connected to SIC) */
enum {
/*0*/	SOFTINT2 = 0,	/* Software interrupt from secondary controller */
/*1*/	MMCI0B,		/* Multimedia card 0B interrupt */
/*2*/	MMCI1B,		/* Multimedia card 1B interrupt */
/*3*/	KMI0,		/* Activity on keyboard port */
/*4*/	KMI1,		/* Activity on mouse port */
/*5*/	SCI1,		/* Smart Card 1 interface interrupt */
/*6*/	UART3,		/* UART 3 empty or data available */
/*7*/	CHAR_LCD,	/* Character LCD ready for data */
/*8*/	TOUCHSCREEN,	/* Pen down on CLCD touchscreen */
/*9*/	KEYPAD,		/* Key pressed on display keypad */
/*10*/	RESERVED10,	/* NA */
/*11*/	RESERVED11,	/* NA */
/*12*/	RESERVED12,	/* NA */
/*13*/	RESERVED13,	/* NA */
/*14*/	RESERVED14,	/* NA */
/*15*/	RESERVED15,	/* NA */
/*16*/	RESERVED16,	/* NA */
/*17*/	RESERVED17,	/* NA */
/*18*/	RESERVED18,	/* NA */
/*19*/	RESERVED19,	/* NA */
/*20*/	RESERVED20,	/* NA */
/*21*/	DISKONCHIP,	/* Interrupt from DiskOnChip flash memory controller */
/*22*/	MMCI0A,		/* Multimedia card 0A interrupt */
/*23*/	MMCI1A,		/* Multimedia card 1A interrupt */
/*24*/	AACI,		/* Audio CODEC interface interrupt */
/*25*/	ETHERNET,	/* Ethernet controller ready for data or data available */
/*26*/	USB,		/* USB controller ready for data or data available */
/*27*/	PCI0,		/* Interrupt 0 triggered from external PCI bus */
/*28*/	PCI1,		/* Interrupt 1 triggered from external PCI bus */
/*29*/	PCI2,		/* Interrupt 2 triggered from external PCI bus */
/*30*/	PCI3,		/* Interrupt 3 triggered from external PCI bus */
/*31*/	RESERVED31,	/* NA */
};


#endif /* ASM_FILE */
#endif /* PL190 */
