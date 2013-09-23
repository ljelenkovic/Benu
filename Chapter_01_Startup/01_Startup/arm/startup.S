/*! startup.S - starting point of control */

.extern	print_hello

.section .text

.global arch_start

arch_start:
	LDR sp, =stack
	BL print_hello
	B .

.section .bss
.align  4

	.lcomm _stack_, 4096
stack:
