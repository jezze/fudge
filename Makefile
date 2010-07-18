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

all: kernel initrd cd

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

i386:
	@echo "Building i386..."
	@cd kernel/arch/i386; make

initrd: lib
	@echo "Creating ramdisk..."
	@cd ramdisk; make
	@$(GCC) -O2 tools/mkinitrd.c -o tools/mkinitrd
	$(MKINITRD) ramdisk/about.txt about.txt ramdisk/cat cat ramdisk/cpu cpu ramdisk/date date ramdisk/echo echo ramdisk/hello hello ramdisk/help.txt help.txt ramdisk/ls ls ramdisk/reboot reboot ramdisk/shell shell ramdisk/timer timer

kernel: lib i386
	@echo "Building kernel..."
	@cd kernel; make
	@$(LD) $(LDFLAGS) \
    lib/call.o \
    lib/calls.o \
    lib/cbuffer.o \
    lib/io.o \
    lib/memory.o \
    lib/stack.o \
    lib/string.o \
    lib/vfs.o \
    kernel/arch/i386/cr.o \
    kernel/arch/i386/gdt.o \
    kernel/arch/i386/idt.o \
    kernel/arch/i386/irq.o \
    kernel/arch/i386/isr.o \
    kernel/arch/i386/loader.o \
    kernel/assert.o \
    kernel/gdt.o \
    kernel/idt.o \
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

lib:
	@echo "Building library..."
	@cd lib; make

