/*! i8253 counter (timer device) */
#ifdef I8253

#include "i8253.h"

#include "../interrupt.h"
#include "../io.h"

#include <kernel/errno.h>

/*! timer device i8253, wrapper for arch_timer_t interface */
arch_timer_t i8253 = (arch_timer_t)
{
	.min_interval = {0, 0},
	.max_interval = {0, 0},
	.init = i8253_init,
	.set_interval = i8253_set_time_to_counter,
	.get_interval_remainder = i8253_get_time_from_counter,
	.enable_interrupt = i8253_enable_interrupt,
	.disable_interrupt = i8253_disable_interrupt,
	.register_interrupt = i8253_register_interrupt
};
/* accessed from 'arch' layer via: extern arch_timer_t i8253 */

/*! Calculate min and max counting interval, and set initial counter */
static void i8253_init()
{
	COUNT_TO_TIME(COUNT_MIN, &i8253.min_interval);
	COUNT_TO_TIME(COUNT_MAX, &i8253.max_interval);

	i8253_set(COUNT_MAX);
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

/*! Load counter with number equivalent to 'time' */
static void i8253_set_time_to_counter(timespec_t *time)
{
	uint cnt;

	ASSERT(time && time->tv_sec == 0 &&
		 time->tv_nsec <= i8253.max_interval.tv_nsec &&
		 time->tv_nsec >= i8253.min_interval.tv_nsec);

	TIME_TO_COUNT(time, cnt);

	i8253_set(cnt);
}

/*! Read current value from counter and convert it into 'time' */
static void i8253_get_time_from_counter(timespec_t *time)
{
	uint cnt;

	ASSERT(time);

	cnt = i8253_get();

	COUNT_TO_TIME(cnt, time);
}

/*! Enable counter interrupts */
static void i8253_enable_interrupt()
{
	arch_irq_enable(IRQ_TIMER);
}

/*! Disable counter interrupts */
static void i8253_disable_interrupt()
{
	arch_irq_disable(IRQ_TIMER);
}

/*! Register function for counter interrupts */
static void i8253_register_interrupt(void *handler)
{
	arch_register_interrupt_handler(IRQ_TIMER, handler);
}

#endif /* I8253 */
