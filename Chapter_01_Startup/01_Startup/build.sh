#!/bin/sh

# usage: ./build.sh [qemu|clean]
#

PROJECT=hello

COMPILE=0

#Compile if no arguments are passed or if "qemu" is passed
if [ $# -eq 0 ]; then
	COMPILE=1;
#... or if "qemu" is passed
elif [ $1 = "qemu" ]; then
	COMPILE=1;
fi

if [ $COMPILE -eq 1 ]; then

CFLAGS="-O3 -m32 -Wall -ffreestanding -nostdlib -fno-stack-protector"
LDFLAGS="-O3 -melf_i386 -e arch_start -Ttext=0x100000"

#compile
gcc -c startup.S $CFLAGS
gcc -c hello.c $CFLAGS

#link
ld startup.o hello.o -o $PROJECT.elf $LDFLAGS

#make iso image with grub boot loader

#create cd directory structure
BOOTCD=cd
GRUBMENU=$BOOTCD/boot/grub/menu.lst
GRUBFILE=$BOOTCD/boot/grub/stage2_eltorito
GRUBFILE_ORIG=grub_file

if [ ! -e $BOOTCD ]; then mkdir -p $BOOTCD/boot/grub ; fi

#build grub menu (which automatically start compiled program)
echo "default 0" > $GRUBMENU
echo "timeout=0" >> $GRUBMENU
echo "title $PROJECT" >> $GRUBMENU
echo "root (cd)" >> $GRUBMENU
echo "kernel /boot/$PROJECT.elf" >> $GRUBMENU
echo "boot" >> $GRUBMENU

cp -a $GRUBFILE_ORIG $GRUBFILE
cp $PROJECT.elf $BOOTCD/boot/$PROJECT.elf

#from directory structure create iso image
mkisofs -J -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 \
	-boot-info-table -V $PROJECT -A $PROJECT -o $PROJECT.iso $BOOTCD \
	2> /dev/null

echo Created ISO CD image: $PROJECT.iso

fi

if [ $# -eq 0 ]; then
DUMMY=
elif [ $1 = "qemu" ]; then

	echo Starting...
	qemu-system-i386 -m 2 -no-kvm -cdrom $PROJECT.iso

elif [ $1 = "clean" ]; then

	echo Cleaning...
	rm -rf *.o *.elf $PROJECT.iso cd

fi
