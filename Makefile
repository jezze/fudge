ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T./linker-x86.ld

ARM_ASM=arm-elf-as
ARM_ASMFLAGS=
ARM_GCC=arm-elf-gcc
ARM_GCCFLAGS=-c -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -std=gnu99 -mhard-float
ARM_LD=arm-elf-ld
ARM_LDFLAGS=-T./linker-arm.ld

ISO=genisoimage
ISOFLAGS=-R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table 
MKINITRD=./tools/mkinitrd

.PHONY: all cd clean x86 initrd kernel lib

all: system-x86
system-arm: arch-arm
system-x86: lib kernel arch-x86 initrd cd

arch-arm:
	@echo "Building ARM..."
	@$(ARM_GCC) $(ARM_GCCFLAGS) kernel/arch/arm/arch.c -o kernel/arch/arm/arch.o
	@$(ARM_ASM) $(ARM_ASMFLAGS) kernel/arch/arm/loader.s -o kernel/arch/arm/loader.o
	@$(ARM_GCC) $(ARM_GCCFLAGS) lib/string.c -o lib/string.o
	@$(ARM_GCC) $(ARM_GCCFLAGS) lib/vfs.c -o lib/vfs.o
	@$(ARM_GCC) $(ARM_GCCFLAGS) kernel/assert.c -o kernel/assert.o
	@$(ARM_GCC) $(ARM_GCCFLAGS) kernel/kernel.c -o kernel/kernel.o
	@$(ARM_LD) $(ARM_LDFLAGS) lib/string.o lib/vfs.o kernel/arch/arm/arch.o kernel/arch/arm/loader.o kernel/assert.o kernel/kernel.o -o root/boot/kernel

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
	@echo "Creating fudge.iso..."
	@$(ISO) $(ISOFLAGS) -o fudge.iso root

clean:
	@echo "Cleaning..."
	@rm -f fudge.img
	@rm -f fudge.iso
	@rm -f root/boot/kernel
	@rm -f root/boot/initrd
	@rm -f tools/mkinitrd
	@cd kernel; make clean
	@cd kernel/arch/arm; make clean
	@cd kernel/arch/x86; make clean
	@cd lib; make clean
	@cd ramdisk; make clean

img:
	@echo "Creating fudge.img..."
	@dd if=/dev/zero of=fudge.img bs=512 count=8000
	@dd if=root/boot/grub/stage1 conv=notrunc of=fudge.img bs=512 seek=0
	@dd if=root/boot/grub/stage2 conv=notrunc of=fudge.img bs=512 seek=1
	@dd if=root/boot/kernel conv=notrunc of=fudge.img bs=512 seek=200
	@dd if=root/boot/initrd conv=notrunc of=fudge.img bs=512 seek=300

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

