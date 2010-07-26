ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T./linker.ld
ISO=genisoimage
ISOFLAGS=-R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table 
MKINITRD=./tools/mkinitrd

.PHONY: all cd clean x86 initrd kernel lib

all: system-x86
system-arm: lib kernel arch-arm initrd
system-x86: lib kernel arch-x86 initrd cd

arch-arm: lib kernel

arch-x86: lib kernel
	@echo "Building x86..."
	@cd kernel/arch/x86; make
	@$(LD) $(LDFLAGS) \
    lib/call.o \
    lib/calls.o \
    lib/cbuffer.o \
    lib/memory.o \
    lib/stack.o \
    lib/string.o \
    lib/vfs.o \
    kernel/arch/x86/arch.o \
    kernel/arch/x86/cr.o \
    kernel/arch/x86/gdt.o \
    kernel/arch/x86/gdts.o \
    kernel/arch/x86/idt.o \
    kernel/arch/x86/idts.o \
    kernel/arch/x86/io.o \
    kernel/arch/x86/irqs.o \
    kernel/arch/x86/isrs.o \
    kernel/arch/x86/loader.o \
    kernel/arch/x86/irq.o \
    kernel/arch/x86/isr.o \
    kernel/arch/x86/kbd.o \
    kernel/arch/x86/mmu.o \
    kernel/arch/x86/pit.o \
    kernel/arch/x86/rtc.o \
    kernel/arch/x86/screen.o \
    kernel/arch/x86/syscall.o \
    kernel/assert.o \
    kernel/initrd.o \
    kernel/kernel.o \
    kernel/mboot.o \
    kernel/shell.o \
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
	@cd kernel/arch/x86; make clean
	@cd lib; make clean
	@cd ramdisk; make clean

initrd: lib
	@echo "Building ramdisk..."
	@cd ramdisk; make
	@echo "Creating ramdisk..."
	@$(GCC) -O2 tools/mkinitrd.c -o tools/mkinitrd
	@$(MKINITRD) ramdisk/about.txt about.txt ramdisk/cat cat ramdisk/cpu cpu ramdisk/date date ramdisk/echo echo ramdisk/elf elf ramdisk/hello hello ramdisk/help.txt help.txt ramdisk/ls ls ramdisk/reboot reboot ramdisk/shell shell ramdisk/timer timer

kernel:
	@echo "Building kernel..."
	@cd kernel; make

lib:
	@echo "Building library..."
	@cd lib; make

