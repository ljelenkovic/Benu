/*! Print on serial port using ARM PrimeCell UART (PL011) */

#ifdef PL011

#include <types/basic.h>
#include <ARCH/config.h>

/*      register	offset     register escription */
#define UART_DR		0x000	/* Data register */
#define UART_RSR	0x004	/* Receive status register */
#define UART_ECR	0x004	/* Error clear register */
#define UART_FR		0x018	/* Flag register */
#define UART_IBRD	0x024	/* Integer baud rate register */
#define UART_FBRD	0x028	/* Fractional baud rate register */
#define UART_LCR_H	0x02C	/* Line control register */
#define UART_CR		0x030	/* Control register */
#define UART_IFLS	0x034	/* Interrupt FIFO level select register */
#define UART_IMSC	0x038	/* Interrupt mask set/clear register */
#define UART_RIS	0x03C	/* Raw interrupt status register */
#define UART_MIS	0x040	/* Masked interrupt status register */
#define UART_ICR	0x044	/* Interrupt clear register */
#define UART_DMACR	0x048	/* DMA control register */

#define DR_ERR_MASK	0x0f00

/* boaud rate calculation */
#define UART_HZ		24000000	/* uart input frequency (assuming 24 MHz) */
#define UART_BIT_RATE	115200		/* desired bit rate */

#define UART_IBRD_V	(UART_HZ /(16 * UART_BIT_RATE))
#define UART_FBRD_V	\
((uint)((1. * UART_HZ /(16. * UART_BIT_RATE) - UART_IBRD_V) * 64 + 0.5))

#define UART_LCR_H_WL	0x60	/* 8 bit word */
#define UART_LCR_H_FEN	0x10	/* Enable FIFO */

#define UART_CR_RXE	0x20	/* Receive enable */
#define UART_CR_TXE	0x10	/* Transmit enable */
#define UART_CR_UARTEN	0x01	/* UART enable */

#define UART_IMSC_TXIM	0x20	/* Transmit interrupt */
#define UART_IMSC_RXIM	0x10	/* Receive interrupt */

#define UART_FR_TXFF	(1<<5)	/* Transmit FIFO full */
#define UART_FR_RXFE	(1<<4)	/* Receive FIFO empty */


/*! Software buffers */
#define BUFFER_SIZE	256	/* software buffer size */


/* parameters for configuring serial port (for future implementations) */
typedef struct _uart_t_
{
	int   speed;		/* baud rate				*/
	int8  data_bits;	/* from 5 to 8				*/
	int8  parity;		/* PARITY_+ NONE/ODD/EVEN/MARK/SPACE	*/
	int8  stop_bit;		/* STOPBIT_1 or STOPBIT_15		*/
	int8  mode;		/* UART_BYTE or UART_STREAM		*/
}
uart_t;

typedef struct _arch_uart_t_
{
	uint    base;

	uart_t  params; /* not used */

	uint8   *inbuff;
	int     inbufsz, inf, inl, insz;
	uint8   *outbuff;
	int     outbufsz, outf, outl, outsz;
}
arch_uart_t;

#define INC_MOD(X,MOD)	do {(X) = ((X)+1 < MOD ?(X)+1 : 0); } while (0)

#endif /* PL011 */
