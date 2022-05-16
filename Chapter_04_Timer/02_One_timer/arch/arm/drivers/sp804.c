/*! sp804 counter (timer device) */
#ifdef SP804

#include "sp804.h"

#include <arch/interrupt.h>
#include <kernel/errno.h>

static void sp804_init();
static void sp804_set(uint cnt);
static uint sp804_get();
static void sp804_enable_interrupt();
static void sp804_disable_interrupt();
static void sp804_register_interrupt(void *handler);
static void sp804_set_time_to_counter(timespec_t *time);
static void sp804_get_time_from_counter(timespec_t *time);
static void (*sp804_handler)();


/*! timer device sp804, wrapper for arch_timer_t interface */
arch_timer_t sp804 = (arch_timer_t)
{
	.min_interval = {0, 0},
	.max_interval = {0, 0},
	.init = sp804_init,
	.set_interval = sp804_set_time_to_counter,
	.get_interval_remainder = sp804_get_time_from_counter,
	.enable_interrupt = sp804_enable_interrupt,
	.disable_interrupt = sp804_disable_interrupt,
	.register_interrupt = sp804_register_interrupt
};
/* accessed from 'arch' layer via: extern arch_timer_t sp804 */

/*! Calculate min and max counting interval, and set initial counter */
static void sp804_init()
{
	volatile uint32 *ptr;

	sp804_handler = NULL;

	COUNT_TO_TIME(ISP804_COUNT_MIN, &sp804.min_interval);
	COUNT_TO_TIME(ISP804_COUNT_MAX, &sp804.max_interval);

	/* disable interrupt generation on timer1 */
	ptr = (uint32 *)(TIMER1_BASE + TIMER_CONTROL);
	*ptr = *ptr &(~TIMER_INT_ENABLE);

	/* set parameters for timer0 */
	ptr = (uint32 *)(TIMER0_BASE + TIMER_CONTROL);
	*ptr = TIMER_SIZE_32;
	sp804_set(ISP804_COUNT_MAX);
	*ptr = TIMER_ENABLE | TIMER_MODE_PERIODIC | TIMER_SIZE_32;
	/* just don't generate interrupts yet */

	/* enable interrupt on VIC */
	arch_irq_enable(TIMER01 + IRQ_OFFSET);
}

/*! Load sp804 counter with 'cnt' */
static void sp804_set(uint cnt)
{
	volatile uint32 *ptr;

	ptr = (uint32 *)(TIMER0_BASE + TIMER_LOAD);
	*ptr = cnt;
}

/*! Read sp804 counter (its current value) */
static uint sp804_get()
{
	volatile uint32 *ptr;

	ptr = (uint32 *)(TIMER0_BASE + TIMER_VALUE);

	return *ptr;
}

/*! Load counter with number equivalent to 'time' */
static void sp804_set_time_to_counter(timespec_t *time)
{
	uint cnt;

	ASSERT(time && time_cmp(time, &sp804.max_interval) <= 0 &&
		time_cmp(time, &sp804.min_interval) >= 0);

	TIME_TO_COUNT(time, cnt);

	sp804_set(cnt);
}

/*! Read current value from counter and convert it into 'time' */
static void sp804_get_time_from_counter(timespec_t *time)
{
	uint cnt;

	ASSERT(time);

	cnt = sp804_get();

	COUNT_TO_TIME(cnt, time);
}

/*! Enable counter interrupts */
static void sp804_enable_interrupt()
{
	volatile uint32 *ptr;

	/* enable interrupt generation on timer0 */
	ptr = (uint32 *)(TIMER0_BASE + TIMER_CONTROL);
	*ptr = *ptr | TIMER_INT_ENABLE;
}

/*! Disable counter interrupts */
static void sp804_disable_interrupt()
{
	volatile uint32 *ptr;

	/* disable interrupt generation on timer0 */
	ptr = (uint32 *)(TIMER0_BASE + TIMER_CONTROL);
	*ptr = *ptr &(~TIMER_INT_ENABLE);
}

/* internal timer handler */
static void sp804_interrupt_handler()
{
	volatile uint32 *ptr;

	/* clear interrupt request */
	ptr = (uint32 *)(TIMER0_BASE + TIMER_INTCLR);
	*ptr = 0;

	if (sp804_handler)
		sp804_handler();
}

/*! Register function for counter interrupts */
static void sp804_register_interrupt(void *handler)
{
	sp804_handler = handler;
	arch_register_interrupt_handler(TIMER01 + IRQ_OFFSET,
					  sp804_interrupt_handler);
}

#endif /* SP804 */
