# Building system script (for 'make')

# valid targets: (all), clean, cleanall, qemu, debug_qemu, debug_gdb
# valid command line defines: debug=yes, optimize=yes

#default target
ARCH ?= i386

CONFIG_INI = arch/$(ARCH)/config.ini
THIS_MAKEFILE = Makefile
CONFIG_FILES = $(CONFIG_INI) $(THIS_MAKEFILE)


include $(CONFIG_INI)

KERNEL_FILE_NAME = kernel.elf
KERNEL_IMG = $(BUILDDIR)/$(KERNEL_FILE_NAME)
PROGS := progs
BUILD_U := $(BUILDDIR)/$(PROGS)
PROGS_BIN := $(addprefix $(BUILD_U)/,$(addsuffix .bin,$(PROGRAMS)))
PROGS_BIN_ALL := $(BUILD_U)/$(PROGS).bin

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
# Threads

CMACROS += SYSTEM_MEMORY=$(SYSTEM_MEMORY)			\
	PRIO_LEVELS=$(PRIO_LEVELS)				\
	THR_DEFAULT_PRIO=$(THR_DEFAULT_PRIO)			\
	KERNEL_STACK_SIZE=$(KERNEL_STACK_SIZE)			\
	DEFAULT_THREAD_STACK_SIZE=$(DEFAULT_THREAD_STACK_SIZE)	\
	HANDLER_STACK_SIZE=$(HANDLER_STACK_SIZE)

#------------------------------------------------------------------------------
CMACROS += $(OPTIONALS)
#------------------------------------------------------------------------------

all: $(KERNEL_IMG) $(PROGS_BIN_ALL)

# Create $(BUILDDIR) and ARCH symbolic link for selected platform source
# (used for #include <ARCH/*> purposes)
BDIR_RDY = $(BUILDDIR)/.null
$(BDIR_RDY):
	@mkdir -p $(BUILDDIR)
	@ln -s ../arch/$(ARCH) $(BUILDDIR)/ARCH
	@touch $(BDIR_RDY)


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Compiling and linking kernel
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BUILD_K = $(BUILDDIR)/kernel

CMACROS_K += $(CMACROS) ASSERT_H=\<kernel/errno.h\> \
		K_INIT_PROG=\"$(K_INIT_PROG)\"

#------------------------------------------------------------------------------
# Memory allocators: 'gma' and/or 'first_fit'
CMACROS_K += MEM_ALLOCATOR_FOR_KERNEL=$(MEM_ALLOCATOR_FOR_KERNEL) \
	LOAD_ADDR=$(LOAD_ADDR)

#------------------------------------------------------------------------------
FILES_K := $(foreach DIR,$(DIRS_K),$(wildcard $(DIR)/*.c $(DIR)/*.S))
OBJS_K := $(addprefix $(BUILD_K)/,$(FILES_K:.c=.o))
OBJS_K := $(OBJS_K:.S=.asm.o)
DEPS_K := $(OBJS_K:.o=.d)

# dummy file that indicate directories for kernel objects are created
KDIRS_CREATED = $(BUILD_K)/.null

# create required directories in $(BUILD_K) directory (including $(BUILD_K))
$(KDIRS_CREATED): $(BDIR_RDY)
	@-if [ ! -e $(BUILD_K) ]; then mkdir -p $(BUILD_K); fi;
	@-$(foreach DIR,$(DIRS_K), if [ ! -e $(BUILD_K)/$(DIR) ]; \
		then mkdir -p $(BUILD_K)/$(DIR); fi; )
	@touch $(KDIRS_CREATED)
	@echo
	@echo ">>> Compiling kernel ..."

# define how to compile .c files
$(BUILD_K)/%.o: %.c $(CONFIG_FILES) $(KDIRS_CREATED)
	@echo [compiling 'kernel'] $< ...
	@$(CC_K) -c $< -o $@ -MMD $(CFLAGS_K) \
		$(foreach INC,$(INCLUDES_K),-I $(INC)) \
		$(foreach MACRO,$(CMACROS_K),-D $(MACRO))

# define how to compile .S files (assembler)
$(BUILD_K)/%.asm.o: %.S $(CONFIG_FILES) $(KDIRS_CREATED)
	@echo [compiling 'kernel'] $< ...
	@$(CC_K) -c $< -o $@ -MMD $(CFLAGS_K) \
		$(foreach INC,$(INCLUDES_K),-I$(INC)) \
		$(foreach MACRO,$(CMACROS_K),-D $(MACRO))

# preprocessed linker script (constants)
LDSCRIPT_KPP := $(BUILD_K)/ldscript.ld
$(LDSCRIPT_KPP): $(LDSCRIPT_K) $(CONFIG_FILES) $(KDIRS_CREATED)
	@$(CC_K) -E -P -x c -o $@ $< $(CFLAGS_K)		\
		$(foreach INC,$(INCLUDES_K),-I$(INC)) 		\
		$(foreach MACRO,$(CMACROS_K),-D $(MACRO))

# KERNEL image
$(KERNEL_IMG): $(OBJS_K) $(LDSCRIPT_KPP)
	@echo [linking 'kernel'] $@
	@$(LINK_K) -o$@ $(OBJS_K) $(LDFLAGS_K) -T $(LDSCRIPT_KPP)

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Compiling and linking programs
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CMACROS_U += $(CMACROS) ASSERT_H=\<api/errno.h\>

#------------------------------------------------------------------------------
# Memory
CMACROS_U += MEM_ALLOCATOR_FOR_USER=$(MEM_ALLOCATOR_FOR_USER)	\
	     MAX_USER_DESCRIPTORS=$(MAX_USER_DESCRIPTORS)

#------------------------------------------------------------------------------

# Programs compilation through template ----------------------------------------

# Template is parsed twice:
# 1) when "called" - when expanded on every "call" location;
#    all "simple" variables and functions are evaluated
#    (variables and functions defined with single $ character)
#    Other ("complex") variables and function are left (but one $ is removed)
# 2) when "final" makefile is parsed


# Template is called with: $(call PROGRAM_TEMPLATE,prog_name)
# for example: $(call PROGRAM_TEMPLATE,hello) => $(1) is hello
define PROGRAM_TEMPLATE

$(1)_HEAP_SIZE := $(word 1,$($(1)))
$(1)_STACK_SIZE := $(word 2,$($(1)))
$(1)_THREAD_STACK_SIZE := $(word 3,$($(1)))
$(1)_INIT := $(word 4,$($(1)))

$(1)_MACROS := $(CMACROS_U) HEAP_SIZE=$$($(1)_HEAP_SIZE)	\
		STACK_SIZE=$$($(1)_STACK_SIZE)			\
		THREAD_STACK_SIZE=$$($(1)_THREAD_STACK_SIZE)	\
		PROG_START_FUNC=$$($(1)_INIT)			\
		PROG_NAME="\"$(1)\""		\
		PROG_HELP=$(1)_prog_help_msg

$(1)_DIRS := $(wordlist 5,$(words $($(1))),$($(1))) $(DIRS_U)

$(1)_FILES := $$(foreach DIR,$$($(1)_DIRS),$$(wildcard $$(DIR)/*.c $$(DIR)/*.S))
$(1)_BUILDDIR := $(BUILD_U)/$(1)
$(1)_BDIRS    := $$(addprefix $$($(1)_BUILDDIR)/,$$($(1)_DIRS))
$(1)_OBJS     := $$(addprefix $$($(1)_BUILDDIR)/,$$($(1)_FILES))
$(1)_OBJS     := $$($(1)_OBJS:.c=.o)
$(1)_OBJS     := $$($(1)_OBJS:.S=.asm.o)
$(1)_DEPS     := $$($(1)_OBJS:.o=.d)
$(1)_TARGET   := $(BUILD_U)/$(1).bin

OBJS_U        += $$($(1)_OBJS)
DEPS_U        += $$($(1)_DEPS)
#PROGRAMS_BIN  += $$($(1)_TARGET)

# dummy file that indicate directories are created
$(1)_DIRS_CREATED := $$($(1)_BUILDDIR)/.null

#create directories for objects
$$($(1)_DIRS_CREATED): $(BDIR_RDY)
	@if [ ! -e $$($(1)_BUILDDIR) ]; then mkdir -p $$($(1)_BUILDDIR); fi;
	@$$(foreach DIR,$$($(1)_BDIRS), if [ ! -e $$(DIR) ]; \
		then mkdir -p $$(DIR); fi; )
	@touch $$($(1)_DIRS_CREATED)
	@echo
	@echo ">>> Compiling program >>> $(1) <<< ..."

#define how to compile .c files
$$($(1)_BUILDDIR)/%.o: %.c $(CONFIG_FILES) $$($(1)_DIRS_CREATED) $(PROGRAM_HEADERS)
	@echo [compiling '$(1)'] $$< ...
	@$$(CC_U) -c $$< -o $$@ -MMD $$(CFLAGS_U) \
		$$(foreach INC,$$(INCLUDES_U),-I $$(INC)) \
		$$(foreach MACRO,$$($(1)_MACROS),-D $$(MACRO))

#define how to compile .S files (assembler)
$$($(1)_BUILDDIR)/%.asm.o: %.S $(CONFIG_FILES) $$($(1)_DIRS_CREATED) $(PROGRAM_HEADERS)
	@echo [compiling '$(1)'] $$< ...
	@$$(CC_U) -c $$< -o $$@ -MMD $$(CFLAGS_U) \
		$$(foreach INC,$$(INCLUDES_U),-I$$(INC)) \
		$$(foreach MACRO,$$($(1)_MACROS),-D $$(MACRO))

# preprocessed linker script
$(1)_LDSCRIPT := $$($(1)_BUILDDIR)/ldscript.ld
$$($(1)_LDSCRIPT): $(LDSCRIPT_U)
	@$$(CC_U) -E -P -x c -o $$@ $$< $$(CFLAGS_U) \
		$$(foreach INC,$$(INCLUDES_U),-I$$(INC)) \
		$$(foreach MACRO,$$($(1)_MACROS),-D $$(MACRO))

#target
$$($(1)_TARGET): $$($(1)_DIRS_CREATED) $$($(1)_OBJS) $$($(1)_LDSCRIPT)
	@echo [linking '$(1)'] $$@
	@$$(LINK_U) -o $$@ $$($(1)_OBJS) $$(LDFLAGS_U) -T $$($(1)_LDSCRIPT)

endef

# "Call" above template for each program to be included
$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_TEMPLATE,$(prog))))

$(PROGS_BIN_ALL): $(PROGS_BIN)
	@cat $(PROGS_BIN) > $(PROGS_BIN_ALL)

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

# starting compiled system in 'qemu' emulator
qemu: $(KERNEL_IMG) $(PROGS_BIN_ALL)
	@echo $(QMSG)
	@$(QEMU) $(QFLAGS) -kernel $(KERNEL_IMG) -initrd "$(PROGS_BIN_ALL)"

OBJECTS = $(OBJS_K) $(OBJS_U)
DEPS = $(DEPS_K) $(DEPS_U)

clean:
	@echo Cleaning.
	@-rm -f $(OBJECTS) $(DEPS) $(KERNEL_IMG) $(PROGS_BIN)

clean_all cleanall:
	@echo Removing build directory!
	@-rm -rf $(BUILDDIR)

-include $(DEPS)
