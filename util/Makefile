# For creating CD ISO image of system:
# - put compiled kernel file (.elf) in cd-image/boot directory as kernel.elf
# - if modules must be loaded gzip them and put in cd-image/boot directory,
#   update menu.lst file from cd-image/boot/grub directory (instruction are there)
# - run "make bootcd" (or just "make")
#
# For running CD ISO image in QEMU:
# - run make qemu

PROJECT = "System name"
CDIMAGE = system.iso
BOOTCD = cd_image

bootcd: $(CDIMAGE)

$(CDIMAGE):
	mkisofs -J -R -b boot/grub/stage2_eltorito -input-charset utf-8 \
	-no-emul-boot -boot-load-size 4 -boot-info-table -V $(PROJECT)	\
	-A $(PROJECT) -o $(CDIMAGE) $(BOOTCD)

# QEMU emulation
ARCH ?= i386
QEMU_MEM = 8
QEMU = qemu-system-$(ARCH)
QFLAGS = -m $(QEMU_MEM)M -machine accel=tcg -serial stdio
QMSG = "Starting qemu (pop-up window)"

qemu: $(CDIMAGE)
	@echo $(QMSG)
	@$(QEMU) $(QFLAGS) -cdrom $(CDIMAGE)

clean:
	-rm $(CDIMAGE)
