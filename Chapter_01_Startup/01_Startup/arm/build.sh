#!/bin/sh

# usage: ./build.sh [qemu|clean]
#
# arm adaptation started with examples on:
#http://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/
#

PROJECT=hello.elf

#Compile if required
if [ $# -eq 0 ] || ( [ $1 = "qemu" ] && [ ! -e $PROJECT ] ); then

CCFLAGS="-mcpu=arm926ej-s -g -Wall -Werror"
ASFLAGS="-mcpu=arm926ej-s -g"
LDFLAGS="-e arch_start -Ttext=0x10000"

CC=arm-none-eabi-gcc
AS=arm-none-eabi-as
LD=arm-none-eabi-ld

#compile
$AS -c startup.S -o startup.o $ASFLAGS
$CC -c hello.c -o hello.o $CCFLAGS

#link
$LD startup.o hello.o -o $PROJECT $LDFLAGS

#if an error occured => exit
if [ ! $? -eq 0 ] ; then
	exit
fi

echo Created System image: $PROJECT

fi #"compile"

if [ $# -gt 0 ] && [ $1 = "qemu" ]; then
	echo Starting... \(press Ctrl+a then x to stop\)
	qemu-system-arm -M versatilepb -m 4M -nographic -kernel $PROJECT

elif [ $# -gt 0 ] && [ $1 = "cleanall" ]; then
	echo Cleaning...
	rm -rf *.o $PROJECT
fi
