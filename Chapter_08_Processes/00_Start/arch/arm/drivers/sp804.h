/*! sp804 counter (timer device) - included from only sp804.c ! */
#ifdef SP804

#pragma once

#include <ARCH/time.h>
#include <ARCH/config.h>

#define	ISP804_FREQ	1000000 /* counter frequency, 1 MHz */
#define N1E9		1000000000L

#define ISP804_COUNT_MAX	((uint32) 0xffffffff) /* (2^32-1)/10^6 s */
#define ISP804_COUNT_MIN	100 /* 100 us */

/* Calculate time from counter value */
#define COUNT_TO_TIME(C, T)						\
do {									\
	(T)->tv_sec = (C) / ISP804_FREQ;				\
	(T)->tv_nsec = ((C) % ISP804_FREQ) * (N1E9 / ISP804_FREQ);	\
} while (0)

/* Calculate counter value from time */
#define TIME_TO_COUNT(T, C)						 \
do {									 \
(C) = (T)->tv_sec * ISP804_FREQ + (T)->tv_nsec /(N1E9 / ISP804_FREQ) ;\
} while (0)


/*! Using timer0 only */

/*!     Register	base offset */
#define TIMER_LOAD	0x00000000
#define TIMER_VALUE	0x00000004
#define TIMER_CONTROL	0x00000008
#define TIMER_INTCLR	0x0000000c
#define TIMER_RIS	0x00000010
#define TIMER_MIS	0x00000014
#define TIMER_BGLOAD	0x00000018

/*! Timer controll register */
#define TIMER_ENABLE		(1<<7)	/* enable timer (counting) */
#define TIMER_MODE_PERIODIC	(1<<6)	/* periodic */
#define TIMER_INT_ENABLE	(1<<5)	/* enable timer interrupt */
/* bit 4 - reserved */
/* bits 3-2: prescale (divisor): 0-1;1-16;2->256;3-not in use*/
#define TIMER_SIZE_32		(1<<1)	/* 32-bit */
#define TIMER_ONESHOT		(1<<0)	/* one shot */

#endif /* SP804 */
