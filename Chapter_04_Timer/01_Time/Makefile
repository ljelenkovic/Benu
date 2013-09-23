# Building system script (for 'make')

# valid targets: (all), clean, cleanall, qemu, debug_qemu, debug_gdb
# valid command line defines: debug=yes, optimize=yes

#default target
PLATFORM ?= i386

CONFIG_INI = arch/$(PLATFORM)/config.ini
THIS_MAKEFILE = Makefile
CONFIG_FILES = $(CONFIG_INI) $(THIS_MAKEFILE)


include $(CONFIG_INI)

KERNEL_FILE_NAME = $(PROJECT).elf
BINFILE = $(BUILDDIR)/$(KERNEL_FILE_NAME)

CMACROS += OS_NAME="\"$(OS_NAME)\"" PROJECT="\"$(PROJECT)\"" 		\
	   NAME_MAJOR="\"$(NAME_MAJOR)\"" NAME_MINOR="\"$(NAME_MINOR)\""\
	   PLATFORM="\"$(PLATFORM)\"" AUTHOR="\"$(AUTHOR)\"" 		\
	   VERSION="\"$(VERSION)\""


#------------------------------------------------------------------------------
# Devices

CMACROS += $(DEVICES) IC_DEV=$(IC_DEV) TIMER=$(TIMER)			\
	K_INITIAL_STDOUT=$(K_INITIAL_STDOUT) K_STDOUT=$(K_STDOUT)	\
	U_STDOUT=$(U_STDOUT) U_STDERR=$(U_STDERR)			\
	TURN_OFF=$(TURN_OFF)

#------------------------------------------------------------------------------
# Misc

CMACROS += STACK_SIZE=$(STACK_SIZE)					\
	LOAD_ADDR=$(LOAD_ADDR)						\
	ASSERT_H=\<kernel/errno.h\>					\
	MEM_ALLOCATOR=$(MEM_ALLOCATOR)

#------------------------------------------------------------------------------
CMACROS += $(OPTIONALS)
#------------------------------------------------------------------------------

all: $(BINFILE)

#create ARCH symbolic link for selected platform (placed in arch/ directory)
#(used only for #include <ARCH/*> purposes)
prepare_src:
	@-if [ ! -d $(BUILDDIR)/ARCH ]; then				\
		mkdir -p $(BUILDDIR);					\
		ln -s ../arch/$(PLATFORM) $(BUILDDIR)/ARCH ;		\
	fi;


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

# preprocessed linker script (konstants)
LDSCRIPT_PP := $(BUILDDIR)/ldscript.ld
$(LDSCRIPT_PP): $(LDSCRIPT) $(CONFIG_FILES)
	@$(CC) -E -P -x c -o $@ $< $(CFLAGS)			\
		$(foreach INC,$(INCLUDES),-I$(INC)) 		\
		$(foreach MACRO,$(CMACROS),-D $(MACRO))


# OS image
$(BINFILE): prepare_src $(OBJECTS) $(LDSCRIPT_PP)
	@echo [linking $@]
	@$(LINK) -o $@ $(OBJECTS) -T $(LDSCRIPT_PP) $(LDFLAGS)


#find qemu emulator requested in config.ini (QEMU)
#ifeq ($(strip $(shell command -v $(QEMU);)),)
#	QEMU = echo "Error: $(QEMU) not found!"; exit ;
#endif

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
