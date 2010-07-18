ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T./linker.ld
ISO=genisoimage
ISOFLAGS=-R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table 
MKINITRD=./tools/mkinitrd

.PHONY: all cd clean i386 initrd kernel lib

all: system-i386
system-arm: lib kernel arch-arm initrd
system-i386: lib kernel arch-i386 initrd cd

arch-arm: lib kernel

arch-i386: lib kernel
	@echo "Building i386..."
	@cd kernel/arch/i386; make
	@$(LD) $(LDFLAGS) \
    lib/call.o \
    lib/calls.o \
    lib/cbuffer.o \
    lib/memory.o \
    lib/stack.o \
    lib/string.o \
    lib/vfs.o \
    kernel/arch/i386/cr.o \
    kernel/arch/i386/gdt.o \
    kernel/arch/i386/gdts.o \
    kernel/arch/i386/idt.o \
    kernel/arch/i386/idts.o \
    kernel/arch/i386/io.o \
    kernel/arch/i386/irqs.o \
    kernel/arch/i386/isrs.o \
    kernel/arch/i386/loader.o \
    kernel/assert.o \
    kernel/initrd.o \
    kernel/irq.o \
    kernel/isr.o \
    kernel/kbd.o \
    kernel/kernel.o \
    kernel/mboot.o \
    kernel/mmu.o \
    kernel/pit.o \
    kernel/rtc.o \
    kernel/screen.o \
    kernel/shell.o \
    kernel/syscall.o \
    -o root/boot/kernel

cd:
	@echo "Creating iso..."
	@$(ISO) $(ISOFLAGS) -o fudge.iso root

clean:
	@echo "Cleaning..."
	@rm -f fudge.iso
	@rm -f root/boot/kernel
	@rm -f root/boot/initrd
	@rm -f tools/mkinitrd
	@cd kernel; make clean
	@cd kernel/arch/i386; make clean
	@cd lib; make clean
	@cd ramdisk; make clean

initrd: lib
	@echo "Building ramdisk..."
	@cd ramdisk; make
	@echo "Creating ramdisk..."
	@$(GCC) -O2 tools/mkinitrd.c -o tools/mkinitrd
	@$(MKINITRD) ramdisk/about.txt about.txt ramdisk/cat cat ramdisk/cpu cpu ramdisk/date date ramdisk/echo echo ramdisk/hello hello ramdisk/help.txt help.txt ramdisk/ls ls ramdisk/reboot reboot ramdisk/shell shell ramdisk/timer timer

kernel:
	@echo "Building kernel..."
	@cd kernel; make

lib:
	@echo "Building library..."
	@cd lib; make

