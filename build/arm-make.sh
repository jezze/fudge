#!/bin/sh

ASM=arm-elf-as
ASMFLAGS=
GCC=arm-elf-gcc
GCCFLAGS=-c -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -std=gnu99 -mhard-float
LD=arm-elf-ld
LDFLAGS=-T./linker-arm.ld

DIR="../kernel"

$GCC $GCCFLAGS $DIR/assert.c -o $DIR/assert.o
$GCC $GCCFLAGS $DIR/kernel.c -o $DIR/kernel.o

DIR="../kernel/arch/arm"

$GCC $GCCFLAGS $DIR/arch.c -o $DIR/arch.o
$ASM $ASMFLAGS $DIR/init.s -o $DIR/init.o

DIR="../lib"

$GCC $GCCFLAGS $DIR/string.c -o $DIR/string.o
$GCC $GCCFLAGS $DIR/vfs.c -o $DIR/vfs.o

$LD $LDFLAGS \
    ../lib/string.o \
    ../lib/vfs.o \
    ../kernel/arch/arm/arch.o \
    ../kernel/arch/arm/init.o \
    ../kernel/assert.o \
    ../kernel/kernel.o \
    -o root/boot/kernel

