# Building system script (for 'make')

# valid targets: (all), clean, cleanall, qemu, debug_qemu, debug_gdb
# valid command line defines: debug=yes, optimize=yes

include conf/config.ini

KERNEL_FILE_NAME = $(PROJECT).elf
BINFILE = $(BUILDDIR)/$(KERNEL_FILE_NAME)
CDIMAGE = $(BUILDDIR)/$(PROJECT).iso

CMACROS = OS_NAME="\"$(OS_NAME)\"" PROJECT="\"$(PROJECT)\"" 		\
	  NAME_MAJOR="\"$(NAME_MAJOR)\"" NAME_MINOR="\"$(NAME_MINOR)\""	\
	  PLATFORM="\"$(PLATFORM)\"" AUTHOR="\"$(AUTHOR)\"" 		\
	  VERSION="\"$(VERSION)\""


#------------------------------------------------------------------------------
# Devices

CMACROS += $(DEVICES) IC_DEV=$(IC_DEV) TIMER=$(TIMER)			\
	K_INITIAL_STDOUT=$(K_INITIAL_STDOUT) K_STDOUT=$(K_STDOUT)	\
	U_STDOUT=$(U_STDOUT) U_STDERR=$(U_STDERR)

#------------------------------------------------------------------------------
# Misc

CMACROS += STACK_SIZE=$(STACK_SIZE)				\
	LOAD_ADDR=$(LOAD_ADDR)			\
	ASSERT_H=\<kernel/errno.h\>				\
	MEM_ALLOCATOR=$(MEM_ALLOCATOR)

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
#if in command line given: debug=yes or/and optimize=yes
ifeq ($(debug),yes)
CFLAGS += -g
CMACROS += DEBUG
ifeq ($(optimize),yes) #if both are set!
CFLAGS += $(CFLAGS_OPTD)
LDFLAGS += $(LDFLAGS_OPTD)
endif
else #optimize by default
CFLAGS += $(CFLAGS_OPT)
LDFLAGS += $(LDFLAGS_OPT)
endif


#------------------------------------------------------------------------------
FILES := $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c $(DIR)/*.S))
OBJECTS := $(addprefix $(BUILDDIR)/,$(FILES:.c=.o))
OBJECTS := $(OBJECTS:.S=.asm.o)
DEPS := $(OBJECTS:.o=.d)

# dummy file that indicate directories for objects are created
DIRS_CREATED = $(BUILDDIR)/.null

# create required directories in $(BUILDDIR) directory (including $(BUILDDIR))
$(DIRS_CREATED):
	@-if [ ! -e $(BUILDDIR) ]; then mkdir -p $(BUILDDIR); fi;
	@-$(foreach DIR,$(DIRS), if [ ! -e $(BUILDDIR)/$(DIR) ];	\
		then mkdir -p $(BUILDDIR)/$(DIR); fi; )
	@touch $(DIRS_CREATED)

# define how to compile .c files
$(BUILDDIR)/%.o: %.c conf/config.ini conf/Makefile $(DIRS_CREATED)
	@echo [compiling] $< ...
	@$(CC) -c $< -o $@ -MMD $(CFLAGS) \
		$(foreach INC,$(INCLUDES),-I $(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO))

# define how to compile .S files (assembler)
$(BUILDDIR)/%.asm.o: %.S conf/config.ini conf/Makefile $(DIRS_CREATED)
	@echo [compiling] $< ...
	@$(CC) -c $< -o $@ -MMD $(CFLAGS) \
		$(foreach INC,$(INCLUDES),-I$(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO))

# preprocessed linker script (konstants)
LDSCRIPT_PP := $(BUILDDIR)/ldscript.ld
$(LDSCRIPT_PP): $(LDSCRIPT)
	@$(CC) -E -P -x c -o $@ $< $(CFLAGS)			\
		$(foreach INC,$(INCLUDES),-I$(INC)) 		\
		$(foreach MACRO,$(CMACROS),-D $(MACRO))


# OS image
$(BINFILE): $(OBJECTS) $(LDSCRIPT_PP)
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
