/*! i8253 counter (timer device) - included from only i8253.c ! */
#ifdef I8253

#pragma once

#include "../time.h"
#include <kernel/time.h>
#include <types/bits.h>

#define	I8253_FREQ	1193180 /* counter frequency */
#define N1E9		1000000000L

#define COUNT_MAX	0xffff
#define COUNT_MIN	(COUNT_MAX >> 10)

/* Calculate time from counter value */
#define COUNT_TO_TIME(C, T)	\
do {(T)->tv_sec = 0;(T)->tv_nsec = mul_div_32(C, N1E9, I8253_FREQ); } while (0)
/* generally would be:
	T.tv_sec = C / I8253_FREQ; but is zero because max (C) < I8253_FREQ!
	T.tv_nsec = (C % I8253_FREQ) * 1.E9 / I8253_FREQ; */

/* Calculate counter value from time */
#define TIME_TO_COUNT(T, C) \
do { C = mul_div_32((T)->tv_nsec, I8253_FREQ, N1E9); } while (0)
/* generally would be:(T->tv_sec + T->tv_nsec / 1.E9) * I8253_FREQ, but since T->tv_sec
   must be zero its little simpler: T->tv_nsec * I8253_FREQ / 1.E9 */


/* i8253 ports and commands */
#define	I8253_CH0	0x40
#define	I8253_CH1	0x41	/* channel 1 not used */
#define	I8253_CH2	0x42	/* channel 2 not used */
#define	I8253_CMD	0x43
#define	I8253_CMD_LOAD	0x34
#define	I8253_CMD_LATCH	0x04

static void i8253_init();
static void i8253_set(uint cnt);
static uint i8253_get();
static void i8253_enable_interrupt();
static void i8253_disable_interrupt();

static void i8253_register_interrupt(void *handler);

static void i8253_set_time_to_counter(timespec_t *time);
static void i8253_get_time_from_counter(timespec_t *time);

#endif /* I8253 */
