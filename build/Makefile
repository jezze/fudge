#!/bin/sh

ARCH=x86

DIR_IMAGE=root
DIR_ARCH=../arch/${ARCH}
DIR_SOURCE_BIN=../bin
DIR_SOURCE_KERNEL=../kernel
DIR_SOURCE_LIB=../lib
DIR_SOURCE_MODULES=../modules

ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
GCCFLAGS_RAMDISK=-c -O2 -I../include/lib -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T${DIR_ARCH}/linker.ld
LDFLAGS_RAMDISK=-e main

.PHONY: lib arch-x86 modules kernel ramdisk image iso clean

all: ramdisk

lib:
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_LIB}/file.c -o ${DIR_SOURCE_LIB}/file.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_LIB}/memory.c -o ${DIR_SOURCE_LIB}/memory.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_LIB}/string.c -o ${DIR_SOURCE_LIB}/string.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/lib/calls.s -o ${DIR_ARCH}/lib/calls.o

arch-x86: lib
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/kernel/arch.c -o ${DIR_ARCH}/kernel/arch.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/kernel/cpu.s -o ${DIR_ARCH}/kernel/cpu.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/kernel/fpu.c -o ${DIR_ARCH}/kernel/fpu.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/kernel/fpus.s -o ${DIR_ARCH}/kernel/fpus.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/kernel/gdt.c -o ${DIR_ARCH}/kernel/gdt.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/kernel/gdts.s -o ${DIR_ARCH}/kernel/gdts.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/kernel/idt.c -o ${DIR_ARCH}/kernel/idt.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/kernel/idts.s -o ${DIR_ARCH}/kernel/idts.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/kernel/init.s -o ${DIR_ARCH}/kernel/init.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/kernel/irq.c -o ${DIR_ARCH}/kernel/irq.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/kernel/irqs.s -o ${DIR_ARCH}/kernel/irqs.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/kernel/isr.c -o ${DIR_ARCH}/kernel/isr.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/kernel/isrs.s -o ${DIR_ARCH}/kernel/isrs.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/kernel/mmu.c -o ${DIR_ARCH}/kernel/mmu.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/kernel/syscall.c -o ${DIR_ARCH}/kernel/syscall.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/kernel/tss.c -o ${DIR_ARCH}/kernel/tss.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/kernel/tsss.s -o ${DIR_ARCH}/kernel/tsss.o

modules: lib
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/elf/elf.c -o ${DIR_SOURCE_MODULES}/elf/elf.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/tty/tty.c -o ${DIR_SOURCE_MODULES}/tty/tty.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/modules/ata/ata.c -o ${DIR_ARCH}/modules/ata/ata.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/modules/io/io.c -o ${DIR_ARCH}/modules/io/io.o
	@${ASM} ${ASMFLAGS} ${DIR_ARCH}/modules/io/ios.s -o ${DIR_ARCH}/modules/io/ios.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/modules/kbd/kbd.c -o ${DIR_ARCH}/modules/kbd/kbd.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/modules/pci/pci.c -o ${DIR_ARCH}/modules/pci/pci.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/modules/pit/pit.c -o ${DIR_ARCH}/modules/pit/pit.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/modules/rtc/rtc.c -o ${DIR_ARCH}/modules/rtc/rtc.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/modules/serial/serial.c -o ${DIR_ARCH}/modules/serial/serial.o
	@${GCC} ${GCCFLAGS} ${DIR_ARCH}/modules/vga/vga.c -o ${DIR_ARCH}/modules/vga/vga.o

kernel: arch-${ARCH} modules
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/initrd.c -o ${DIR_SOURCE_KERNEL}/initrd.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/kernel.c -o ${DIR_SOURCE_KERNEL}/kernel.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/log.c -o ${DIR_SOURCE_KERNEL}/log.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/modules.c -o ${DIR_SOURCE_KERNEL}/modules.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/shell.c -o ${DIR_SOURCE_KERNEL}/shell.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/vfs.c -o ${DIR_SOURCE_KERNEL}/vfs.o
	@${LD} ${LDFLAGS} \
    ${DIR_SOURCE_KERNEL}/initrd.o \
    ${DIR_SOURCE_KERNEL}/kernel.o \
    ${DIR_SOURCE_KERNEL}/log.o \
    ${DIR_SOURCE_KERNEL}/modules.o \
    ${DIR_SOURCE_KERNEL}/shell.o \
    ${DIR_SOURCE_KERNEL}/vfs.o \
    ${DIR_SOURCE_MODULES}/elf/elf.o \
    ${DIR_SOURCE_MODULES}/tty/tty.o \
    ${DIR_ARCH}/kernel/arch.o \
    ${DIR_ARCH}/kernel/cpu.o \
    ${DIR_ARCH}/kernel/fpu.o \
    ${DIR_ARCH}/kernel/fpus.o \
    ${DIR_ARCH}/kernel/gdt.o \
    ${DIR_ARCH}/kernel/gdts.o \
    ${DIR_ARCH}/kernel/idt.o \
    ${DIR_ARCH}/kernel/idts.o \
    ${DIR_ARCH}/kernel/init.o \
    ${DIR_ARCH}/kernel/irqs.o \
    ${DIR_ARCH}/kernel/isrs.o \
    ${DIR_ARCH}/kernel/irq.o \
    ${DIR_ARCH}/kernel/isr.o \
    ${DIR_ARCH}/kernel/mmu.o \
    ${DIR_ARCH}/kernel/syscall.o \
    ${DIR_ARCH}/kernel/tss.o \
    ${DIR_ARCH}/kernel/tsss.o \
    ${DIR_ARCH}/modules/ata/ata.o \
    ${DIR_ARCH}/modules/io/io.o \
    ${DIR_ARCH}/modules/io/ios.o \
    ${DIR_ARCH}/modules/kbd/kbd.o \
    ${DIR_ARCH}/modules/pci/pci.o \
    ${DIR_ARCH}/modules/pit/pit.o \
    ${DIR_ARCH}/modules/rtc/rtc.o \
    ${DIR_ARCH}/modules/serial/serial.o \
    ${DIR_ARCH}/modules/vga/vga.o \
    ${DIR_SOURCE_LIB}/memory.o \
    ${DIR_SOURCE_LIB}/file.o \
    ${DIR_SOURCE_LIB}/string.o \
    ${DIR_ARCH}/lib/calls.o \
    -o root/boot/kernel

ramdisk: kernel
	@cp ${DIR_ARCH}/modules/ata/ata.o ${DIR_IMAGE}/lib/modules/ata.ko
	@cp ${DIR_ARCH}/modules/io/io.o ${DIR_IMAGE}/lib/modules/io.ko
	@cp ${DIR_ARCH}/modules/kbd/kbd.o ${DIR_IMAGE}/lib/modules/kbd.ko
	@cp ${DIR_ARCH}/modules/pci/pci.o ${DIR_IMAGE}/lib/modules/pci.ko
	@cp ${DIR_ARCH}/modules/pit/pit.o ${DIR_IMAGE}/lib/modules/pit.ko
	@cp ${DIR_ARCH}/modules/rtc/rtc.o ${DIR_IMAGE}/lib/modules/rtc.ko
	@cp ${DIR_ARCH}/modules/serial/serial.o ${DIR_IMAGE}/lib/modules/serial.ko
	@cp ${DIR_ARCH}/modules/vga/vga.o ${DIR_IMAGE}/lib/modules/vga.ko
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/cat.c -o ${DIR_SOURCE_BIN}/cat.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/cd.c -o ${DIR_SOURCE_BIN}/cd.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/clear.c -o ${DIR_SOURCE_BIN}/clear.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_BIN}/cpu.s -o ${DIR_SOURCE_BIN}/cpus.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/cpu.c -o ${DIR_SOURCE_BIN}/cpu.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/date.c -o ${DIR_SOURCE_BIN}/date.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/echo.c -o ${DIR_SOURCE_BIN}/echo.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/elf.c -o ${DIR_SOURCE_BIN}/elf.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/hello.c -o ${DIR_SOURCE_BIN}/hello.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/ls.c -o ${DIR_SOURCE_BIN}/ls.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/reboot.c -o ${DIR_SOURCE_BIN}/reboot.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/shell.c -o ${DIR_SOURCE_BIN}/shell.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/tar.c -o ${DIR_SOURCE_BIN}/tar.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/timer.c -o ${DIR_SOURCE_BIN}/timer.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/vga.c -o ${DIR_SOURCE_BIN}/vga.o
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/cat.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/cat
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/cd.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/cd
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/clear.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/clear
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/cpu.o ${DIR_SOURCE_BIN}/cpus.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/cpu
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/date.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/date
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/echo.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/echo
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/elf.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/elf
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/hello.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/hello
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/ls.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/ls
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/reboot.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/reboot
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/shell.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/shell
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/tar.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/tar
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/timer.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/timer
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/vga.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_ARCH}/lib/calls.o -o ${DIR_IMAGE}/bin/vga
	@tar -cvf initrd.tar ${DIR_IMAGE}
	@find ${DIR_IMAGE} -depth -print | cpio -ov > initrd.cpio
	@mv initrd.tar ${DIR_IMAGE}/boot
	@mv initrd.cpio ${DIR_IMAGE}/boot
    

image:
	@dd if=/dev/zero of=fudge.img bs=512 count=2880
	@dd if=root/boot/grub/stage1 conv=notrunc of=fudge.img bs=512 seek=0
	@dd if=root/boot/grub/stage2 conv=notrunc of=fudge.img bs=512 seek=1
	@dd if=menu.lst conv=notrunc of=fudge.img bs=512 seek=200
	@dd if=root/boot/kernel conv=notrunc of=fudge.img bs=512 seek=300
	@dd if=root/boot/initrd.tar conv=notrunc of=fudge.img bs=512 seek=400
	@sh x86-write-image.sh

iso:
	@genisoimage -R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table -o fudge.iso root

clean:
	@rm -f fudge.img
	@rm -f fudge.iso
	@rm -f root/lib/modules/*.ko
	@rm -f root/bin/cat
	@rm -f root/bin/cd
	@rm -f root/bin/clear
	@rm -f root/bin/cpu
	@rm -f root/bin/date
	@rm -f root/bin/echo
	@rm -f root/bin/elf
	@rm -f root/bin/hello
	@rm -f root/bin/ls
	@rm -f root/bin/reboot
	@rm -f root/bin/shell
	@rm -f root/bin/tar
	@rm -f root/bin/timer
	@rm -f root/bin/vga
	@rm -f root/boot/kernel
	@rm -f root/boot/initrd
	@rm -f root/boot/initrd.tar
	@rm -f root/boot/initrd.cpio
	@rm -f ../arch/arm/kernel/*.o
	@rm -f ../arch/arm/lib/*.o
	@rm -f ../arch/x86/kernel/*.o
	@rm -f ../arch/x86/lib/*.o
	@rm -f ../arch/x86/modules/ata/*.o
	@rm -f ../arch/x86/modules/io/*.o
	@rm -f ../arch/x86/modules/kbd/*.o
	@rm -f ../arch/x86/modules/pci/*.o
	@rm -f ../arch/x86/modules/pit/*.o
	@rm -f ../arch/x86/modules/rtc/*.o
	@rm -f ../arch/x86/modules/serial/*.o
	@rm -f ../arch/x86/modules/vga/*.o
	@rm -f ../bin/*.o
	@rm -f ../kernel/*.o
	@rm -f ../lib/*.o
	@rm -f ../modules/elf/*.o
	@rm -f ../modules/tty/*.o

