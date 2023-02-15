/*! Multiboot configuration */
/* source: http://www.gnu.org/software/grub/manual/multiboot/multiboot.html */

#pragma once

/* 'Instructions' to multiboot loader */

/* Align all boot modules on i386 page (4KB) boundaries. */
#define MULTIBOOT_PAGE_ALIGN	0x00000001
/* Must pass memory information to OS. */
#define MULTIBOOT_MEMORY_INFO	0x00000002

/* The magic field should contain this. */
#define MULTIBOOT_HEADER_MAGIC	0x1BADB002
#define MULTIBOOT_HEADER_FLAGS	(MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO)

/* Flags to be set in the 'flags' member of the multiboot info structure
   (boot loader sets this flags upon loading kernel). */

/* This should be in %eax. */
#define MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002

/* Since other multiboot options aren't used they are not present here */
