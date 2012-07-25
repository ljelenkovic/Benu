# Building system script (for 'make')

# valid targets: (all), clean, cleanall, qemu, debug_qemu, debug_gdb
# valid command line defines: debug=yes, optimize=yes

include conf/config.ini

KERNEL_FILE_NAME = $(PROJECT).elf
BINFILE = $(BUILDDIR)/$(KERNEL_FILE_NAME)
CDIMAGE = $(BUILDDIR)/$(PROJECT).iso

CMACROS = OS_NAME="\"$(OS_NAME)\"" PROJECT="\"$(PROJECT)\"" \
	  NAME_MAJOR="\"$(NAME_MAJOR)\"" NAME_MINOR="\"$(NAME_MINOR)\"" \
	  PLATFORM="\"$(PLATFORM)\"" AUTHOR="\"$(AUTHOR)\"" \
	  VERSION="\"$(VERSION)\""


#------------------------------------------------------------------------------
# Devices

#little "magic" for automatically setting DEV_VARS and DEV_PTRS
#with changes only in DEVICES_DEV (for kernel/devices.c):
# DEV_VARS = vga_text_dev,uart_com1,i8042_dev
# DEV_PTRS = &vga_text_dev,&uart_com1,&i8042_dev
comma := ,
empty :=
space := $(empty) $(empty)
DEV_VARS := $(subst $(space),$(comma),$(DEVICES_DEV))
DEV_PTRS := $(addprefix \&,$(DEVICES_DEV))
DEV_PTRS := $(subst $(space),$(comma),$(DEV_PTRS))

CMACROS += $(DEVICES) DEVICES_DEV=$(DEV_VARS) DEVICES_DEV_PTRS=$(DEV_PTRS)   \
	IC_DEV=$(IC_DEV) TIMER=$(TIMER)					     \
	K_INITIAL_STDOUT=$(K_INITIAL_STDOUT) K_STDOUT="\"$(K_STDOUT)\""      \
	U_STDIN="\"$(U_STDIN)\"" U_STDOUT="\"$(U_STDOUT)\"" \
	U_STDERR="\"$(U_STDERR)\""

CMACROS += MAX_RESOURCES=$(MAX_RESOURCES)

#------------------------------------------------------------------------------
# Threads

CMACROS += PRIO_LEVELS=$(PRIO_LEVELS)				\
	THR_DEFAULT_PRIO=$(THR_DEFAULT_PRIO)			\
	KERNEL_STACK_SIZE=$(KERNEL_STACK_SIZE)			\
	DEFAULT_THREAD_STACK_SIZE=$(DEFAULT_THREAD_STACK_SIZE)	\
	PROG_HEAP_SIZE=$(PROG_HEAP_SIZE)			\
	LOAD_ADDR=$(LOAD_ADDR)

#------------------------------------------------------------------------------
CMACROS += $(OPTIONALS)
#------------------------------------------------------------------------------

all: $(CDIMAGE)

#create ARCH symbolic link for selected platform (placed in arch/ directory)
#(used only for #include <ARCH/*> purposes)
prepare_src:
	@-if [ ! -d $(BUILDDIR)/ARCH ]; then			\
		mkdir -p $(BUILDDIR);				\
		ln -s ../arch/$(PLATFORM) $(BUILDDIR)/ARCH ;	\
	fi;

# Linker flags
#if in command line given: debug=yes or/and optimize=yes add flags to LDFLAGS
ifeq ($(debug),yes)
ifeq ($(optimize),yes)
LDFLAGS += $(LDFLAGS_OPTD)
endif
else #optimize by default
LDFLAGS += $(LDFLAGS_OPT)
endif

include conf/Makefile.kernel
# Variables defined in *.kernel: OBJS_K DEPS_K

include conf/Makefile.progs
# Variables defined in *.progs: OBJS_U DEPS_U

OBJECTS = $(OBJS_K) $(OBJS_U)
DEPS = $(DEPS_K) $(DEPS_U)


# OS image
$(BINFILE): $(PROGRAM_HEADERS) $(OBJECTS) $(LDSCRIPT_PP)
	@echo [linking $@]
	@$(LINK) -o $@ $(OBJECTS) $(LDFLAGS) -T $(LDSCRIPT_PP)


BOOTCD := $(BUILDDIR)/cd
GRUBMENU := $(BOOTCD)/boot/grub/menu.lst
GRUBFILE := $(BOOTCD)/boot/grub/stage2_eltorito
GRUBFILE_ORIG := $(BUILDDIR)/ARCH/boot/grub_file

$(GRUBFILE):
	@-if [ ! -e $(BOOTCD) ]; then mkdir -p $(BOOTCD)/boot/grub ; fi;
	@cp -a $(GRUBFILE_ORIG) $(GRUBFILE)

$(GRUBMENU): $(BINFILE)
	@-if [ ! -e $(BOOTCD) ]; then mkdir -p $(BOOTCD)/boot/grub ; fi;
	@echo "default 0" > $(GRUBMENU)
	@echo "timeout=0" >> $(GRUBMENU)
	@echo "title $(PROJECT)" >> $(GRUBMENU)
	@echo "root (cd)" >> $(GRUBMENU)
	@echo "kernel /boot/$(KERNEL_FILE_NAME)" >> $(GRUBMENU)
	@echo "boot" >> $(GRUBMENU)


# ISO CD image for booting (with grub as boot loader and $(KERNEL) as OS image)
$(CDIMAGE): prepare_src $(BINFILE) $(GRUBFILE) $(GRUBMENU)
	@cp $(BINFILE) $(BOOTCD)/boot/$(KERNEL_FILE_NAME)
	@mkisofs -J -R -b boot/grub/stage2_eltorito			\
	-no-emul-boot -boot-load-size 4 -boot-info-table -V $(PROJECT)	\
	-A $(PROJECT) -o $(CDIMAGE) $(BOOTCD) 2> /dev/null
	@echo
	@echo ISO CD image: $(CDIMAGE)
	@echo


#find qemu emulator (qemu or qemu-system-i386 or qemu-system-x86_64)
QEMU =
ifneq ($(strip $(shell command -v qemu;)),)
	QEMU = qemu
else ifneq ($(strip $(shell command -v qemu-system-i386;)),)
	QEMU = qemu-system-i386
else ifneq ($(strip $(shell command -v qemu-system-x86_64;)),)
	QEMU = qemu-system-x86_64
else
	QEMU = echo "Error: qemu not found!"; exit ;
endif

# starting compiled system in 'qemu' emulator
qemu: all
	@echo Starting qemu ...
	@$(QEMU) -m 8 -no-kvm -cdrom $(CDIMAGE) -serial stdio

# DEBUGGING
# For debugging to work: include '-g' in CFLAGS and omit -s and -S from LDFLAGS
# Best if -O3 flag is also omitted from CFLAGS and LDFLAGS (or some variables
# may be optimized away)
# Start debugging from two consoles:
#	1st: make debug_qemu
#	2nd: make debug_gdb
debug_qemu: all
	@echo Starting qemu ...
	@$(QEMU) -m 8 -s -S -no-kvm -cdrom $(CDIMAGE) -serial stdio
debug_gdb: all
	@echo Starting gdb ...
	@gdb -s $(BINFILE) -ex 'target remote localhost:1234'

clean:
	@echo Cleaning.
	@-rm -f $(OBJECTS) $(DEPS) $(CDIMAGE) $(BINFILE)

clean_all cleanall:
	@echo Removing build directory!
	@-rm -rf $(BUILDDIR)

-include $(DEPS)
