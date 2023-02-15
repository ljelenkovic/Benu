/*! interrupt-handlers.c - low level (arch) interrupt handling */
#define _ARCH_INTERRUPTS_HANDLERS_C_

#include "interrupt.h"

void arch_interrupt_handler(int irq_num); /* defined in interrupt.c */

/* Interrupt handlers using GCC facilities for saving context */ 

__attribute__ ((interrupt)) void interrupt_0(struct interrupt_frame *frame)
{
	arch_interrupt_handler(0);
}

__attribute__ ((interrupt)) void interrupt_1(struct interrupt_frame *frame)
{
	arch_interrupt_handler(1);
}

__attribute__ ((interrupt)) void interrupt_2(struct interrupt_frame *frame)
{
	arch_interrupt_handler(2);
}

__attribute__ ((interrupt)) void interrupt_3(struct interrupt_frame *frame)
{
	arch_interrupt_handler(3);
}

__attribute__ ((interrupt)) void interrupt_4(struct interrupt_frame *frame)
{
	arch_interrupt_handler(4);
}

__attribute__ ((interrupt)) void interrupt_5(struct interrupt_frame *frame)
{
	arch_interrupt_handler(5);
}

__attribute__ ((interrupt)) void interrupt_6(struct interrupt_frame *frame)
{
	arch_interrupt_handler(6);
}

__attribute__ ((interrupt)) void interrupt_7(struct interrupt_frame *frame)
{
	arch_interrupt_handler(7);
}

__attribute__ ((interrupt)) void interrupt_8(struct interrupt_frame *frame, arch_word_t error_code)
{
	arch_interrupt_handler(8);
}

__attribute__ ((interrupt)) void interrupt_9(struct interrupt_frame *frame)
{
	arch_interrupt_handler(9);
}

__attribute__ ((interrupt)) void interrupt_10(struct interrupt_frame *frame, arch_word_t error_code)
{
	arch_interrupt_handler(10);
}

__attribute__ ((interrupt)) void interrupt_11(struct interrupt_frame *frame, arch_word_t error_code)
{
	arch_interrupt_handler(11);
}

__attribute__ ((interrupt)) void interrupt_12(struct interrupt_frame *frame, arch_word_t error_code)
{
	arch_interrupt_handler(12);
}

__attribute__ ((interrupt)) void interrupt_13(struct interrupt_frame *frame, arch_word_t error_code)
{
	arch_interrupt_handler(13);
}

__attribute__ ((interrupt)) void interrupt_14(struct interrupt_frame *frame, arch_word_t error_code)
{
	arch_interrupt_handler(14);
}

__attribute__ ((interrupt)) void interrupt_15(struct interrupt_frame *frame)
{
	arch_interrupt_handler(15);
}

__attribute__ ((interrupt)) void interrupt_16(struct interrupt_frame *frame)
{
	arch_interrupt_handler(16);
}

__attribute__ ((interrupt)) void interrupt_17(struct interrupt_frame *frame)
{
	arch_interrupt_handler(17);
}

__attribute__ ((interrupt)) void interrupt_18(struct interrupt_frame *frame)
{
	arch_interrupt_handler(18);
}

__attribute__ ((interrupt)) void interrupt_19(struct interrupt_frame *frame)
{
	arch_interrupt_handler(19);
}

__attribute__ ((interrupt)) void interrupt_20(struct interrupt_frame *frame)
{
	arch_interrupt_handler(20);
}

__attribute__ ((interrupt)) void interrupt_21(struct interrupt_frame *frame)
{
	arch_interrupt_handler(21);
}

__attribute__ ((interrupt)) void interrupt_22(struct interrupt_frame *frame)
{
	arch_interrupt_handler(22);
}

__attribute__ ((interrupt)) void interrupt_23(struct interrupt_frame *frame)
{
	arch_interrupt_handler(23);
}

__attribute__ ((interrupt)) void interrupt_24(struct interrupt_frame *frame)
{
	arch_interrupt_handler(24);
}

__attribute__ ((interrupt)) void interrupt_25(struct interrupt_frame *frame)
{
	arch_interrupt_handler(25);
}

__attribute__ ((interrupt)) void interrupt_26(struct interrupt_frame *frame)
{
	arch_interrupt_handler(26);
}

__attribute__ ((interrupt)) void interrupt_27(struct interrupt_frame *frame)
{
	arch_interrupt_handler(27);
}

__attribute__ ((interrupt)) void interrupt_28(struct interrupt_frame *frame)
{
	arch_interrupt_handler(28);
}

__attribute__ ((interrupt)) void interrupt_29(struct interrupt_frame *frame)
{
	arch_interrupt_handler(29);
}

__attribute__ ((interrupt)) void interrupt_30(struct interrupt_frame *frame)
{
	arch_interrupt_handler(30);
}

__attribute__ ((interrupt)) void interrupt_31(struct interrupt_frame *frame)
{
	arch_interrupt_handler(31);
}

__attribute__ ((interrupt)) void interrupt_32(struct interrupt_frame *frame)
{
	arch_interrupt_handler(32);
}

__attribute__ ((interrupt)) void interrupt_33(struct interrupt_frame *frame)
{
	arch_interrupt_handler(33);
}

__attribute__ ((interrupt)) void interrupt_34(struct interrupt_frame *frame)
{
	arch_interrupt_handler(34);
}

__attribute__ ((interrupt)) void interrupt_35(struct interrupt_frame *frame)
{
	arch_interrupt_handler(35);
}

__attribute__ ((interrupt)) void interrupt_36(struct interrupt_frame *frame)
{
	arch_interrupt_handler(36);
}

__attribute__ ((interrupt)) void interrupt_37(struct interrupt_frame *frame)
{
	arch_interrupt_handler(37);
}

__attribute__ ((interrupt)) void interrupt_38(struct interrupt_frame *frame)
{
	arch_interrupt_handler(38);
}

__attribute__ ((interrupt)) void interrupt_39(struct interrupt_frame *frame)
{
	arch_interrupt_handler(39);
}

__attribute__ ((interrupt)) void interrupt_40(struct interrupt_frame *frame)
{
	arch_interrupt_handler(40);
}

__attribute__ ((interrupt)) void interrupt_41(struct interrupt_frame *frame)
{
	arch_interrupt_handler(41);
}

__attribute__ ((interrupt)) void interrupt_42(struct interrupt_frame *frame)
{
	arch_interrupt_handler(42);
}

__attribute__ ((interrupt)) void interrupt_43(struct interrupt_frame *frame)
{
	arch_interrupt_handler(43);
}

__attribute__ ((interrupt)) void interrupt_44(struct interrupt_frame *frame)
{
	arch_interrupt_handler(44);
}

__attribute__ ((interrupt)) void interrupt_45(struct interrupt_frame *frame)
{
	arch_interrupt_handler(45);
}

__attribute__ ((interrupt)) void interrupt_46(struct interrupt_frame *frame)
{
	arch_interrupt_handler(46);
}

__attribute__ ((interrupt)) void interrupt_47(struct interrupt_frame *frame)
{
	arch_interrupt_handler(47);
}

__attribute__ ((interrupt)) void interrupt_48(struct interrupt_frame *frame)
{
	arch_interrupt_handler(48);
}

void *arch_interrupt_handlers[] = {
	interrupt_0,
	interrupt_1,
	interrupt_2,
	interrupt_3,
	interrupt_4,
	interrupt_5,
	interrupt_6,
	interrupt_7,
	interrupt_8,
	interrupt_9,
	interrupt_10,
	interrupt_11,
	interrupt_12,
	interrupt_13,
	interrupt_14,
	interrupt_15,
	interrupt_16,
	interrupt_17,
	interrupt_18,
	interrupt_19,
	interrupt_20,
	interrupt_21,
	interrupt_22,
	interrupt_23,
	interrupt_24,
	interrupt_25,
	interrupt_26,
	interrupt_27,
	interrupt_28,
	interrupt_29,
	interrupt_30,
	interrupt_31,
	interrupt_32,
	interrupt_33,
	interrupt_34,
	interrupt_35,
	interrupt_36,
	interrupt_37,
	interrupt_38,
	interrupt_39,
	interrupt_40,
	interrupt_41,
	interrupt_42,
	interrupt_43,
	interrupt_44,
	interrupt_45,
	interrupt_46,
	interrupt_47,
	interrupt_48
};
