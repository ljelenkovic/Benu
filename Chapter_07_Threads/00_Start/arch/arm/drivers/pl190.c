/*! PrimeCell Vectored Interrupt Controller (VIC, PL190) */
#ifdef PL190

#include "pl190.h"

#include <ARCH/interrupt.h>
#include <kernel/errno.h>

static void pl190_init();
static void pl190_irq_enable(unsigned int irq);
static void pl190_irq_disable(unsigned int irq);
static void pl190_irq_disable(unsigned int irq);
static void pl190_at_exit(unsigned int irq);
static int  pl190_get_irq();
static char *pl190_interrupt_description(unsigned int n);


/*! interface to arch layer - arch_ic_t */
arch_ic_t pl190 = (arch_ic_t)
{
	.init = pl190_init,
	.disable_irq = pl190_irq_disable,
	.enable_irq = pl190_irq_enable,
	.at_exit = pl190_at_exit,
	.get_irq = pl190_get_irq,
	.int_descr = pl190_interrupt_description
};


/*! Initialize VIC */
static void pl190_init()
{
	volatile uint32 *vicreg;

	/* disable all interrupts */
	vicreg = (void *)(VICBASE + VICINTENABLE);
	*vicreg = 0;

	/* all sources generate IRQ not FIQ */
	vicreg = (void *)(VICBASE + VICINTSELECT);
	*vicreg = 0;

	/* disable "software" interrupts */
	vicreg = (void *)(VICBASE + VICSOFTINT);
	*vicreg = 0;

	/* allow "user mode" to also control interrupts */
	vicreg = (void *)(VICBASE + VICPROTECTION);
	*vicreg = 0;

	/* PIC initialized, all external interrupts disabled */
}

/*!
 * Enable particular external interrupt in VIC
 * \param irq Interrupt request number
 */
static void pl190_irq_enable(unsigned int irq)
{
	volatile uint32 *vicreg;

	ASSERT(irq >= IRQ_OFFSET && irq < INTERRUPTS);

	irq -= IRQ_OFFSET;

	if (irq < 32)
	{
		vicreg = (void *)(VICBASE + VICINTENABLE);
		*vicreg = (*vicreg) |(1 << irq);
	}
	else {
		/* secondary irq controller; TODO */
	}
}

/*!
 * Disable particular external interrupt in VIC
 * \param irq Interrupt request number
 */
static void pl190_irq_disable(unsigned int irq)
{
	volatile uint32 *vicreg;

	ASSERT(irq >= IRQ_OFFSET && irq < INTERRUPTS);

	irq -= IRQ_OFFSET;

	if (irq < 32)
	{
		vicreg = (void *)(VICBASE + VICINTENABLE);
		*vicreg = (*vicreg) &(~(1 << irq));
	}
	else {
		/* secondary irq controller; TODO */
	}
}

/*!
 * At end of interrupt processing, re enable some ports in VIC
 * \param irq Interrupt request number
 */
static void pl190_at_exit(unsigned int irq)
{
}

static int pl190_get_irq()
{
	volatile uint32 *vicreg, mask, irq;

	vicreg = (void *)(VICBASE + VICIRQSTATUS);

	mask = *vicreg;

	if (mask)
	{
		/* if more interrupt request are acitve get one:
		 * with lowest or higest number? */
#if 0	/* return highest number */
		irq = 31 - __builtin_clz(mask) + IRQ_OFFSET;
#else	/* return lowest number */
		irq = __builtin_ffs(mask) - 1 + IRQ_OFFSET;
#endif
	}
	else {
		irq = 0;
	}

	return irq;
}

#ifdef DEBUG
/*! Interrupts descriptions (IRQs) */
static char *arch_irq_desc[INTERRUPTS] =
{
	/* Processor interrupts */
	"Undefined instruction",
	"Software interrupt(by SWI/SVC instructions)",
	"Prefetch abort",
	"Data abort",
	"Interrupt Request",
	"Fast Interrupt Request",

	/* Primary interrupt Controller (VIC) */
	"Watchdog timer",
	"Software interrupt (not one generated with SVC/SWI)",
	"Debug communications receive interrupt",
	"Debug communications transmit interrupt",
	"Timer 0 or 1 Timers on development chip",
	"Timer 2 or 3 Timers on development chip",
	"GPIO controller in development chip",
	"GPIO controller in development chip",
	"GPIO controller in development chip",
	"GPIO controller in development chip",
	"Real time clock in development chip",
	"Synchronous serial port in development chip",
	"UART0 on development chip",
	"UART1 on development chip",
	"UART2 on development chip",
	"Smart Card interface in development chip",
	"CLCD controller in development chip",
	"DMA controller in development chip",
	"Power failure from FPGA",
	"Graphics processor on development chip",
	"Reserved",
	"External",
	"External",
	"External",
	"External",
	"External",
	"External",
	"External",
	"External",
	"External",
	"External",
	"External interrupt from secondary controller",

	/* Secondary interrupt controller (SIC) */
	"Software interrupt from secondary controller",
	"Multimedia card 0B interrupt",
	"Multimedia card 1B interrupt",
	"Activity on keyboard port",
	"Activity on mouse port",
	"Smart Card 1 interface interrupt",
	"UART 3 empty or data available",
	"Character LCD ready for data",
	"Pen down on CLCD touchscreen",
	"Key pressed on display keypad",
	"NA",
	"NA",
	"NA",
	"NA",
	"NA",
	"NA",
	"NA",
	"NA",
	"NA",
	"NA",
	"NA",
	"Interrupt from DiskOnChip flash memory controller",
	"Multimedia card 0A interrupt",
	"Multimedia card 1A interrupt",
	"Audio CODEC interface interrupt",
	"Ethernet controller ready for data or data available",
	"USB controller ready for data or data available",
	"Interrupt 0 triggered from external PCI bus",
	"Interrupt 1 triggered from external PCI bus",
	"Interrupt 2 triggered from external PCI bus",
	"Interrupt 3 triggered from external PCI bus",
	"NA"
};

/*!
 * Return info for requested interrupt number
 * \param n Interrupt number
 * \return Pointer to description string
 */
static char *pl190_interrupt_description(unsigned int n)
{
	if (n < INTERRUPTS)
		return arch_irq_desc[n];
	else
		return "Unknown interrupt number";
}
#else
static char *pl190_interrupt_description(unsigned int n)
{
	return "Descriptions unavailable in this build";
}
#endif

#endif /* PL190 */
