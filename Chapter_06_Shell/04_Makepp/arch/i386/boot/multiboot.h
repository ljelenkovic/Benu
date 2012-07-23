/*! Multiboot configuration */
/* source: http://www.gnu.org/software/grub/manual/multiboot/multiboot.html */

#pragma once

/* 'Instructions' to multiboot loader (grub) */

/* Align all boot modules on i386 page (4KB) boundaries. */
#define MULTIBOOT_PAGE_ALIGN	0x00000001
/* Must pass memory information to OS. */
#define MULTIBOOT_MEMORY_INFO	0x00000002

/* The magic field should contain this. */
#define MULTIBOOT_HEADER_MAGIC	0x1BADB002
#define MULTIBOOT_HEADER_FLAGS	( MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO )


#ifndef ASM_FILE

#include <types/basic.h>

/* Flags to be set in the 'flags' member of the multiboot info structure
   (boot loader (grub) sets this flags upon loading kernel). */

/* This should be in %eax. */
#define MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002

/* is there basic lower/upper memory information? */
#define MULTIBOOT_INFO_MEMORY		0x00000001
/* is there a boot device set? */
#define MULTIBOOT_INFO_BOOTDEV		0x00000002
/* is the command-line defined? */
#define MULTIBOOT_INFO_CMDLINE		0x00000004
/* are there modules to do something with? */
#define MULTIBOOT_INFO_MODS		0x00000008
/* is there a symbol table loaded? */
#define MULTIBOOT_INFO_AOUT_SYMS	0x00000010
/* is there an ELF section header table? */
#define MULTIBOOT_INFO_ELF_SHDR		0X00000020
/* is there a full memory map? */
#define MULTIBOOT_INFO_MEM_MAP		0x00000040
/* Is there drive info? */
#define MULTIBOOT_INFO_DRIVE_INFO	0x00000080
/* Is there a config table? */
#define MULTIBOOT_INFO_CONFIG_TABLE	0x00000100
/* Is there a boot loader name? */
#define MULTIBOOT_INFO_BOOT_LOADER_NAME	0x00000200
/* Is there a APM table? */
#define MULTIBOOT_INFO_APM_TABLE	0x00000400
/* Is there video information? */
#define MULTIBOOT_INFO_VIDEO_INFO	0x00000800

/* The symbol table for a.out. */
typedef struct _multiboot_aout_symbol_table_t_
{
	uint32  tabsize;
	uint32  strsize;
	uint32  addr;
	uint32  reserved;
}
multiboot_aout_symbol_table_t;

/* The section header table for ELF. */
typedef struct _multiboot_elf_section_header_table_t_
{
	uint32  num;
	uint32  size;
	uint32  addr;
	uint32  shndx;
}
multiboot_elf_section_header_table_t;

/* multiboot info provided to OS by boot loader */
typedef struct _multiboot_info_t_
{
	uint32  flags; /* Multiboot info version number */

	/* Available memory from BIOS */
	uint32  mem_lower;
	uint32  mem_upper;

	/* "root" partition */
	uint32  boot_device;

	/* Kernel command line */
	uint32  cmdline;

	/* Boot-Module list */
	uint32  mods_count;
	uint32  mods_addr;

	union
	{
		multiboot_aout_symbol_table_t         aout_sym;
		multiboot_elf_section_header_table_t  elf_sec;
	} u;

	/* Memory Mapping buffer */
	uint32  mmap_length;
	uint32  mmap_addr;

	/* Drive Info buffer */
	uint32  drives_length;
	uint32  drives_addr;

	/* ROM configuration table */
	uint32  config_table;

	/* Boot Loader Name */
	uint32  boot_loader_name;

	/* APM table */
	uint32  apm_table;

	/* Video */
	uint32  vbe_control_info;
	uint32  vbe_mode_info;
	uint16  vbe_mode;
	uint16  vbe_interface_seg;
	uint16  vbe_interface_off;
	uint16  vbe_interface_len;

}
multiboot_info_t;

/* BIOS memory map */
typedef struct multiboot_mmap_entry
{
	uint32  size;
	uint64  addr;
	uint64  len;
	#define MULTIBOOT_MEMORY_AVAILABLE	1
	#define MULTIBOOT_MEMORY_RESERVED	2
	uint32  type;
}
__attribute__((packed)) multiboot_memory_map_t;

/* Module info */
typedef struct _multiboot_module_t_
{
	/* the memory used goes from bytes 'mod_start' to 'mod_end-1' */
	uint32  mod_start;
	uint32  mod_end;

	/* Module command line */
	uint32  cmdline;

	/* padding to take it to 16 bytes (must be zero) */
	uint32  pad;
}
multiboot_module_t;

#endif /* ASM_FILE */
