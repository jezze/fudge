#!/bin/sh

DIR_KERNEL="../kernel"
DIR_ARCH_KERNEL="../arch/arm/kernel"
DIR_ARCH_LIB="../arch/arm/lib"
DIR_LIB="../lib"

ASM="arm-elf-as"
ASMFLAGS=
GCC="arm-elf-gcc"
GCCFLAGS="-c -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -std=gnu99 -mhard-float -static-libgcc -lgcc"
LD="arm-elf-ld"
LDFLAGS="-T./linker-arm.ld"

$GCC $GCCFLAGS $DIR_KERNEL/assert.c -o $DIR_KERNEL/assert.o
$GCC $GCCFLAGS $DIR_KERNEL/dev.c -o $DIR_KERNEL/dev.o
$GCC $GCCFLAGS $DIR_KERNEL/kernel.c -o $DIR_KERNEL/kernel.o
$GCC $GCCFLAGS $DIR_KERNEL/vfs.c -o $DIR_KERNEL/vfs.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/arch.c -o $DIR_ARCH_KERNEL/arch.o
$ASM $ASMFLAGS $DIR_ARCH_KERNEL/init.s -o $DIR_ARCH_KERNEL/init.o
$GCC $GCCFLAGS $DIR_LIB/call.c -o $DIR_LIB/call.o
$GCC $GCCFLAGS $DIR_LIB/memory.c -o $DIR_LIB/memory.o
$GCC $GCCFLAGS $DIR_LIB/stdin.c -o $DIR_LIB/stdin.o
$GCC $GCCFLAGS $DIR_LIB/stdout.c -o $DIR_LIB/stdout.o
$GCC $GCCFLAGS $DIR_LIB/string.c -o $DIR_LIB/string.o
$GCC $GCCFLAGS $DIR_LIB/vfs.c -o $DIR_LIB/vfs.o
$GCC $GCCFLAGS $DIR_ARCH_LIB/call.c -o $DIR_ARCH_LIB/call.o

$LD $LDFLAGS \
    $DIR_KERNEL/assert.o \
    $DIR_KERNEL/dev.o \
    $DIR_KERNEL/kernel.o \
    $DIR_KERNEL/vfs.o \
    $DIR_ARCH_KERNEL/arch.o \
    $DIR_ARCH_KERNEL/init.o \
    $DIR_LIB/memory.o \
    $DIR_LIB/stdin.o \
    $DIR_LIB/stdout.o \
    $DIR_LIB/string.o \
    $DIR_LIB/vfs.o \
    $DIR_ARCH_LIB/call.o \
    -o root/boot/kernel

