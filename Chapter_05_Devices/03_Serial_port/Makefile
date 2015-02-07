# Building system script (for 'make')

# valid targets: (all), clean, cleanall, qemu, debug_qemu, debug_gdb
# valid command line defines: debug=yes, optimize=yes

#default target
ARCH ?= i386

CONFIG_INI = arch/$(ARCH)/config.ini
THIS_MAKEFILE = Makefile
CONFIG_FILES = $(CONFIG_INI) $(THIS_MAKEFILE)


include $(CONFIG_INI)

KERNEL_FILE_NAME = $(PROJECT).elf
BINFILE = $(BUILDDIR)/$(KERNEL_FILE_NAME)

CMACROS += OS_NAME="\"$(OS_NAME)\"" PROJECT="\"$(PROJECT)\"" 		\
	   NAME_MAJOR="\"$(NAME_MAJOR)\"" NAME_MINOR="\"$(NAME_MINOR)\""\
	   ARCH="\"$(ARCH)\"" AUTHOR="\"$(AUTHOR)\"" 		\
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
	U_STDIN="\"$(U_STDIN)\"" U_STDOUT="\"$(U_STDOUT)\"" 		     \
	U_STDERR="\"$(U_STDERR)\""

CMACROS += MAX_RESOURCES=$(MAX_RESOURCES)

#------------------------------------------------------------------------------
# Misc

CMACROS += SYSTEM_MEMORY=$(SYSTEM_MEMORY)			\
	STACK_SIZE=$(STACK_SIZE)				\
	LOAD_ADDR=$(LOAD_ADDR)					\
	ASSERT_H=\<kernel/errno.h\>				\
	MEM_ALLOCATOR=$(MEM_ALLOCATOR)				\
	MAX_USER_DESCRIPTORS=$(MAX_USER_DESCRIPTORS)

#------------------------------------------------------------------------------
CMACROS += $(OPTIONALS)
#------------------------------------------------------------------------------

all: $(BINFILE)

#------------------------------------------------------------------------------
FILES := $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c $(DIR)/*.S))
OBJECTS := $(addprefix $(BUILDDIR)/,$(FILES:.c=.o))
OBJECTS := $(OBJECTS:.S=.asm.o)
DEPS := $(OBJECTS:.o=.d)

# dummy file that indicate directories for objects are created
DIRS_CREATED = $(BUILDDIR)/.null

# create required directories in $(BUILDDIR) directory (including $(BUILDDIR))
# also create ARCH symbolic link for selected platform source
# (used for #include <ARCH/*> purposes)
$(DIRS_CREATED):
	@-if [ ! -e $(BUILDDIR) ]; then mkdir -p $(BUILDDIR); fi;
	@-$(foreach DIR,$(DIRS), if [ ! -e $(BUILDDIR)/$(DIR) ];	\
		then mkdir -p $(BUILDDIR)/$(DIR); fi; )
	@ln -s ../arch/$(ARCH) $(BUILDDIR)/ARCH
	@touch $(DIRS_CREATED)

# define how to compile .c files
$(BUILDDIR)/%.o: %.c $(CONFIG_FILES) $(DIRS_CREATED)
	@echo [compiling] $< ...
	@$(CC) -c $< -o $@ -MMD $(CFLAGS) \
		$(foreach INC,$(INCLUDES),-I $(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO))

# define how to compile .S files (assembler)
$(BUILDDIR)/%.asm.o: %.S $(CONFIG_FILES) $(DIRS_CREATED)
	@echo [compiling] $< ...
	@$(CC) -c $< -o $@ -MMD $(CFLAGS) \
		$(foreach INC,$(INCLUDES),-I$(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO))

# preprocessed linker script (constants)
LDSCRIPT_PP := $(BUILDDIR)/ldscript.ld
$(LDSCRIPT_PP): $(LDSCRIPT) $(CONFIG_FILES) $(DIRS_CREATED)
	@$(CC) -E -P -x c -o $@ $< $(CFLAGS)			\
		$(foreach INC,$(INCLUDES),-I$(INC)) 		\
		$(foreach MACRO,$(CMACROS),-D $(MACRO))


# OS image
$(BINFILE): $(OBJECTS) $(LDSCRIPT_PP)
	@echo [linking $@]
	@$(LINK) -o $@ $(OBJECTS) -T $(LDSCRIPT_PP) $(LDFLAGS)


# starting compiled system in 'qemu' emulator
qemu: all
	@echo $(QMSG)
	@$(QEMU) $(QFLAGS) -kernel $(BINFILE)

# DEBUGGING
# For debugging to work: include '-g' in CFLAGS and omit -s and -S from LDFLAGS
# Best if -O3 flag is also omitted from CFLAGS and LDFLAGS (or some variables
# may be optimized away)
# Start debugging from two consoles:
#	1st: make debug_qemu
#	2nd: make debug_gdb
debug_qemu: all
	@echo $(QMSG)
	@$(QEMU) $(QFLAGS) -kernel $(BINFILE) -s -S
debug_gdb: all
	@echo Starting gdb ...
	@$(DEBUG_GDB) -s $(BINFILE) -ex 'target remote localhost:1234'

clean:
	@echo Cleaning.
	@-rm -f $(OBJECTS) $(DEPS) $(BINFILE)

clean_all cleanall:
	@echo Removing build directory!
	@-rm -rf $(BUILDDIR)

-include $(DEPS)
