# Building system script (for 'make')

# valid targets: (all), clean, cleanall, qemu
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

CMACROS += $(DEVICES) 							\
	K_INITIAL_STDOUT=$(K_INITIAL_STDOUT) K_STDOUT=$(K_STDOUT)	\
	U_STDOUT=$(U_STDOUT) U_STDERR=$(U_STDERR)

#------------------------------------------------------------------------------
# Misc

CMACROS += SYSTEM_MEMORY=$(SYSTEM_MEMORY)				\
	STACK_SIZE=$(STACK_SIZE)					\
	LOAD_ADDR=$(LOAD_ADDR)

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

clean:
	@echo Cleaning.
	@-rm -f $(OBJECTS) $(DEPS) $(BINFILE)

clean_all cleanall:
	@echo Removing build directory!
	@-rm -rf $(BUILDDIR)

-include $(DEPS)
