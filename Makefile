ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T./linker-x86.ld

ARM_ASM=arm-elf-as
ARM_ASMFLAGS=
ARM_GCC=arm-elf-gcc
ARM_GCCFLAGS=-c -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -std=gnu99 -mhard-float
ARM_LD=arm-elf-ld
ARM_LDFLAGS=-T./linker-arm.ld

.PHONY: all clean initrd kernel lib link tools

all: system-x86
system-arm: arch-arm link-arm
system-x86: arch-x86 link-x86 initrd

arch-arm:
	@echo "Building ARM..."
	@$(ARM_GCC) $(ARM_GCCFLAGS) kernel/arch/arm/arch.c -o kernel/arch/arm/arch.o
	@$(ARM_ASM) $(ARM_ASMFLAGS) kernel/arch/arm/init.s -o kernel/arch/arm/init.o
	@$(ARM_GCC) $(ARM_GCCFLAGS) lib/string.c -o lib/string.o
	@$(ARM_GCC) $(ARM_GCCFLAGS) lib/vfs.c -o lib/vfs.o
	@$(ARM_GCC) $(ARM_GCCFLAGS) kernel/assert.c -o kernel/assert.o
	@$(ARM_GCC) $(ARM_GCCFLAGS) kernel/kernel.c -o kernel/kernel.o

arch-x86: lib kernel
	@echo "Building x86..."
	@cd kernel/arch/x86; make

link-arm: arch-arm
	@$(ARM_LD) $(ARM_LDFLAGS) \
    lib/string.o \
    lib/vfs.o \
    kernel/arch/arm/arch.o \
    kernel/arch/arm/init.o \
    kernel/assert.o \
    kernel/kernel.o \
    -o build/root/boot/kernel

link-x86: arch-x86
	@$(LD) $(LDFLAGS) \
    lib/call.o \
    lib/calls.o \
    lib/cbuffer.o \
    lib/memory.o \
    lib/stack.o \
    lib/string.o \
    lib/vfs.o \
    kernel/arch/x86/arch.o \
    kernel/arch/x86/ata.o \
    kernel/arch/x86/cr.o \
    kernel/arch/x86/fpu.o \
    kernel/arch/x86/fpus.o \
    kernel/arch/x86/gdt.o \
    kernel/arch/x86/gdts.o \
    kernel/arch/x86/idt.o \
    kernel/arch/x86/idts.o \
    kernel/arch/x86/init.o \
    kernel/arch/x86/io.o \
    kernel/arch/x86/irqs.o \
    kernel/arch/x86/isrs.o \
    kernel/arch/x86/irq.o \
    kernel/arch/x86/isr.o \
    kernel/arch/x86/kbd.o \
    kernel/arch/x86/mmu.o \
    kernel/arch/x86/pit.o \
    kernel/arch/x86/rtc.o \
    kernel/arch/x86/screen.o \
    kernel/arch/x86/syscall.o \
    kernel/assert.o \
    kernel/elf.o \
    kernel/initrd.o \
    kernel/kernel.o \
    kernel/mboot.o \
    kernel/shell.o \
    -o build/root/boot/kernel

clean:
	@echo "Cleaning..."
	@rm -f build/fudge.img
	@rm -f build/fudge.iso
	@rm -f build/root/boot/kernel
	@rm -f build/root/boot/initrd
	@rm -f tools/mkinitrd
	@cd kernel; make clean
	@cd kernel/arch/arm; make clean
	@cd kernel/arch/x86; make clean
	@cd lib; make clean
	@cd ramdisk; make clean

initrd: lib tools
	@echo "Building ramdisk..."
	@cd ramdisk; make
	@echo "Creating ramdisk..."
	@tools/mkinitrd ramdisk/about.txt about.txt ramdisk/cat cat ramdisk/cpu cpu ramdisk/date date ramdisk/echo echo ramdisk/elf elf ramdisk/hello hello ramdisk/help.txt help.txt ramdisk/ls ls ramdisk/reboot reboot ramdisk/shell shell ramdisk/timer timer

kernel: lib
	@echo "Building kernel..."
	@cd kernel; make

lib:
	@echo "Building library..."
	@cd lib; make

tools:
	@echo "Building tools..."
	@cd tools; make

