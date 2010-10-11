#!/bin/sh

DIR_KERNEL="../kernel"
DIR_ARCH_KERNEL="../arch/x86/kernel"
DIR_ARCH_LIB="../arch/x86/lib"
DIR_LIB="../lib"
DIR_RAMDISK="../ramdisk"
DIR_TOOLS="../tools"

ASM="nasm"
ASMFLAGS="-f elf"
GCC="gcc"
GCCFLAGS="-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs"
LD="ld"
LDFLAGS=-"T./linker-x86.ld"

$GCC $GCCFLAGS $DIR_KERNEL/assert.c -o $DIR_KERNEL/assert.o
$GCC $GCCFLAGS $DIR_KERNEL/dev.c -o $DIR_KERNEL/dev.o
$GCC $GCCFLAGS $DIR_KERNEL/elf.c -o $DIR_KERNEL/elf.o
$GCC $GCCFLAGS $DIR_KERNEL/initrd.c -o $DIR_KERNEL/initrd.o
$GCC $GCCFLAGS $DIR_KERNEL/kernel.c -o $DIR_KERNEL/kernel.o
$GCC $GCCFLAGS $DIR_KERNEL/mboot.c -o $DIR_KERNEL/mboot.o
$GCC $GCCFLAGS $DIR_KERNEL/shell.c -o $DIR_KERNEL/shell.o
$GCC $GCCFLAGS $DIR_KERNEL/vfs.c -o $DIR_KERNEL/vfs.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/arch.c -o $DIR_ARCH_KERNEL/arch.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/ata.c -o $DIR_ARCH_KERNEL/ata.o
$ASM $ASMFLAGS $DIR_ARCH_KERNEL/cr.s -o $DIR_ARCH_KERNEL/cr.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/fpu.c -o $DIR_ARCH_KERNEL/fpu.o
$ASM $ASMFLAGS $DIR_ARCH_KERNEL/fpus.s -o $DIR_ARCH_KERNEL/fpus.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/gdt.c -o $DIR_ARCH_KERNEL/gdt.o
$ASM $ASMFLAGS $DIR_ARCH_KERNEL/gdts.s -o $DIR_ARCH_KERNEL/gdts.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/idt.c -o $DIR_ARCH_KERNEL/idt.o
$ASM $ASMFLAGS $DIR_ARCH_KERNEL/idts.s -o $DIR_ARCH_KERNEL/idts.o
$ASM $ASMFLAGS $DIR_ARCH_KERNEL/init.s -o $DIR_ARCH_KERNEL/init.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/io.c -o $DIR_ARCH_KERNEL/io.o
$ASM $ASMFLAGS $DIR_ARCH_KERNEL/ios.s -o $DIR_ARCH_KERNEL/ios.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/irq.c -o $DIR_ARCH_KERNEL/irq.o
$ASM $ASMFLAGS $DIR_ARCH_KERNEL/irqs.s -o $DIR_ARCH_KERNEL/irqs.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/isr.c -o $DIR_ARCH_KERNEL/isr.o
$ASM $ASMFLAGS $DIR_ARCH_KERNEL/isrs.s -o $DIR_ARCH_KERNEL/isrs.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/kbd.c -o $DIR_ARCH_KERNEL/kbd.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/mmu.c -o $DIR_ARCH_KERNEL/mmu.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/pit.c -o $DIR_ARCH_KERNEL/pit.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/rtc.c -o $DIR_ARCH_KERNEL/rtc.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/screen.c -o $DIR_ARCH_KERNEL/screen.o
$GCC $GCCFLAGS $DIR_ARCH_KERNEL/syscall.c -o $DIR_ARCH_KERNEL/syscall.o
$GCC $GCCFLAGS $DIR_LIB/call.c -o $DIR_LIB/call.o
$GCC $GCCFLAGS $DIR_LIB/cbuffer.c -o $DIR_LIB/cbuffer.o
$GCC $GCCFLAGS $DIR_LIB/memory.c -o $DIR_LIB/memory.o
$GCC $GCCFLAGS $DIR_LIB/stack.c -o $DIR_LIB/stack.o
$GCC $GCCFLAGS $DIR_LIB/string.c -o $DIR_LIB/string.o
$GCC $GCCFLAGS $DIR_LIB/vfs.c -o $DIR_LIB/vfs.o
$ASM $ASMFLAGS $DIR_ARCH_LIB/calls.s -o $DIR_ARCH_LIB/calls.o

$LD $LDFLAGS \
    $DIR_KERNEL/assert.o \
    $DIR_KERNEL/dev.o \
    $DIR_KERNEL/elf.o \
    $DIR_KERNEL/initrd.o \
    $DIR_KERNEL/kernel.o \
    $DIR_KERNEL/mboot.o \
    $DIR_KERNEL/shell.o \
    $DIR_KERNEL/vfs.o \
    $DIR_ARCH_KERNEL/arch.o \
    $DIR_ARCH_KERNEL/ata.o \
    $DIR_ARCH_KERNEL/cr.o \
    $DIR_ARCH_KERNEL/fpu.o \
    $DIR_ARCH_KERNEL/fpus.o \
    $DIR_ARCH_KERNEL/gdt.o \
    $DIR_ARCH_KERNEL/gdts.o \
    $DIR_ARCH_KERNEL/idt.o \
    $DIR_ARCH_KERNEL/idts.o \
    $DIR_ARCH_KERNEL/init.o \
    $DIR_ARCH_KERNEL/io.o \
    $DIR_ARCH_KERNEL/ios.o \
    $DIR_ARCH_KERNEL/irqs.o \
    $DIR_ARCH_KERNEL/isrs.o \
    $DIR_ARCH_KERNEL/irq.o \
    $DIR_ARCH_KERNEL/isr.o \
    $DIR_ARCH_KERNEL/kbd.o \
    $DIR_ARCH_KERNEL/mmu.o \
    $DIR_ARCH_KERNEL/pit.o \
    $DIR_ARCH_KERNEL/rtc.o \
    $DIR_ARCH_KERNEL/screen.o \
    $DIR_ARCH_KERNEL/syscall.o \
    $DIR_LIB/call.o \
    $DIR_LIB/cbuffer.o \
    $DIR_LIB/memory.o \
    $DIR_LIB/stack.o \
    $DIR_LIB/string.o \
    $DIR_LIB/vfs.o \
    $DIR_ARCH_LIB/calls.o \
    -o root/boot/kernel

ASM="nasm"
ASMFLAGS="-f elf"
GCC="gcc"
GCCFLAGS="-c -O2 -I../include/lib -Wall -ffreestanding -nostdlib -nostartfiles -nodefaultlibs"
LD="ld"
LDFLAGS="-Ttext 0x300000 -e main"

$GCC $GCCFLAGS $DIR_RAMDISK/cat.c -o $DIR_RAMDISK/cat.o
$LD $LDFLAGS $DIR_RAMDISK/cat.o $DIR_LIB/memory.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/cat

$ASM $ASMFLAGS $DIR_RAMDISK/cpu.s -o $DIR_RAMDISK/cpus.o
$GCC $GCCFLAGS $DIR_RAMDISK/cpu.c -o $DIR_RAMDISK/cpu.o
$LD $LDFLAGS $DIR_RAMDISK/cpu.o $DIR_RAMDISK/cpus.o $DIR_LIB/memory.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/cpu

$GCC $GCCFLAGS $DIR_RAMDISK/date.c -o $DIR_RAMDISK/date.o
$LD $LDFLAGS $DIR_RAMDISK/date.o $DIR_LIB/memory.o $DIR_LIB/stack.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/date

$GCC $GCCFLAGS $DIR_RAMDISK/echo.c -o $DIR_RAMDISK/echo.o
$LD $LDFLAGS $DIR_RAMDISK/echo.o $DIR_LIB/memory.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/echo

$GCC $GCCFLAGS $DIR_RAMDISK/elf.c -o $DIR_RAMDISK/elf.o
$LD $LDFLAGS $DIR_RAMDISK/elf.o $DIR_LIB/memory.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/elf

$GCC $GCCFLAGS $DIR_RAMDISK/hello.c -o $DIR_RAMDISK/hello.o
$LD $LDFLAGS $DIR_RAMDISK/hello.o $DIR_LIB/memory.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/hello

$GCC $GCCFLAGS $DIR_RAMDISK/ls.c -o $DIR_RAMDISK/ls.o
$LD $LDFLAGS $DIR_RAMDISK/ls.o $DIR_LIB/memory.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/ls

$GCC $GCCFLAGS $DIR_RAMDISK/reboot.c -o $DIR_RAMDISK/reboot.o
$LD $LDFLAGS $DIR_RAMDISK/reboot.o $DIR_LIB/memory.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/reboot

$GCC $GCCFLAGS $DIR_RAMDISK/shell.c -o $DIR_RAMDISK/shell.o
$LD $LDFLAGS $DIR_RAMDISK/shell.o $DIR_LIB/memory.o $DIR_LIB/stack.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/shell

$GCC $GCCFLAGS $DIR_RAMDISK/timer.c -o $DIR_RAMDISK/timer.o
$LD $LDFLAGS $DIR_RAMDISK/timer.o $DIR_LIB/memory.o $DIR_LIB/string.o $DIR_LIB/vfs.o $DIR_LIB/call.o $DIR_ARCH_LIB/calls.o -o $DIR_RAMDISK/timer

GCC="gcc"
GCCFLAGS="-O2"

$GCC $GCCFLAGS $DIR_TOOLS/mkinitrd.c -o $DIR_TOOLS/mkinitrd

$DIR_TOOLS/mkinitrd $DIR_RAMDISK/about.txt about.txt $DIR_RAMDISK/cat cat $DIR_RAMDISK/cpu cpu $DIR_RAMDISK/date date $DIR_RAMDISK/echo echo $DIR_RAMDISK/elf elf $DIR_RAMDISK/hello hello $DIR_RAMDISK/help.txt help.txt $DIR_RAMDISK/ls ls $DIR_RAMDISK/reboot reboot $DIR_RAMDISK/shell shell $DIR_RAMDISK/timer timer

