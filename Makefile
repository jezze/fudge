#!/bin/sh

ARCH=x86

DIR_IMAGE=build/root
DIR_SOURCE_BIN=bin
DIR_SOURCE_KERNEL=kernel
DIR_SOURCE_ARCH=kernel/arch/${ARCH}
DIR_SOURCE_LIB=lib
DIR_SOURCE_MODULES=modules

ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -Iinclude -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
GCCFLAGS_RAMDISK=-c -O2 -Iinclude/lib -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
LD=ld
LDFLAGS=-T${DIR_SOURCE_ARCH}/linker.ld -melf_i386
LDFLAGS_RAMDISK=-e main -melf_i386

.PHONY: lib arch-x86 modules kernel ramdisk image iso clean

all: ramdisk

lib:
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_LIB}/file.c -o ${DIR_SOURCE_LIB}/file.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_LIB}/memory.c -o ${DIR_SOURCE_LIB}/memory.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_LIB}/string.c -o ${DIR_SOURCE_LIB}/string.o

arch-x86: lib
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/arch.c -o ${DIR_SOURCE_ARCH}/arch.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_ARCH}/calls.s -o ${DIR_SOURCE_ARCH}/calls.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_ARCH}/cpu.s -o ${DIR_SOURCE_ARCH}/cpu.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/fpu.c -o ${DIR_SOURCE_ARCH}/fpu.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_ARCH}/fpus.s -o ${DIR_SOURCE_ARCH}/fpus.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/gdt.c -o ${DIR_SOURCE_ARCH}/gdt.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_ARCH}/gdts.s -o ${DIR_SOURCE_ARCH}/gdts.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/idt.c -o ${DIR_SOURCE_ARCH}/idt.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_ARCH}/idts.s -o ${DIR_SOURCE_ARCH}/idts.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_ARCH}/init.s -o ${DIR_SOURCE_ARCH}/init.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/irq.c -o ${DIR_SOURCE_ARCH}/irq.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_ARCH}/irqs.s -o ${DIR_SOURCE_ARCH}/irqs.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/isr.c -o ${DIR_SOURCE_ARCH}/isr.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_ARCH}/isrs.s -o ${DIR_SOURCE_ARCH}/isrs.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/mmu.c -o ${DIR_SOURCE_ARCH}/mmu.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/mboot.c -o ${DIR_SOURCE_ARCH}/mboot.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/syscall.c -o ${DIR_SOURCE_ARCH}/syscall.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_ARCH}/tss.c -o ${DIR_SOURCE_ARCH}/tss.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_ARCH}/tsss.s -o ${DIR_SOURCE_ARCH}/tsss.o

modules: lib
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/ata/ata.c -o ${DIR_SOURCE_MODULES}/ata/ata.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/elf/elf.c -o ${DIR_SOURCE_MODULES}/elf/elf.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/io/io.c -o ${DIR_SOURCE_MODULES}/io/io.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_MODULES}/io/ios.s -o ${DIR_SOURCE_MODULES}/io/ios.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/kbd/kbd.c -o ${DIR_SOURCE_MODULES}/kbd/kbd.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/pci/pci.c -o ${DIR_SOURCE_MODULES}/pci/pci.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/pit/pit.c -o ${DIR_SOURCE_MODULES}/pit/pit.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/rtc/rtc.c -o ${DIR_SOURCE_MODULES}/rtc/rtc.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/rtl8139/rtl8139.c -o ${DIR_SOURCE_MODULES}/rtl8139/rtl8139.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/serial/serial.c -o ${DIR_SOURCE_MODULES}/serial/serial.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/tty/tty.c -o ${DIR_SOURCE_MODULES}/tty/tty.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_MODULES}/vga/vga.c -o ${DIR_SOURCE_MODULES}/vga/vga.o

kernel: arch-${ARCH} modules
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/error.c -o ${DIR_SOURCE_KERNEL}/error.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/initrd.c -o ${DIR_SOURCE_KERNEL}/initrd.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/kernel.c -o ${DIR_SOURCE_KERNEL}/kernel.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/log.c -o ${DIR_SOURCE_KERNEL}/log.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/modules.c -o ${DIR_SOURCE_KERNEL}/modules.o
	@${GCC} ${GCCFLAGS} ${DIR_SOURCE_KERNEL}/vfs.c -o ${DIR_SOURCE_KERNEL}/vfs.o
	@${LD} ${LDFLAGS} \
		${DIR_SOURCE_KERNEL}/error.o \
		${DIR_SOURCE_KERNEL}/initrd.o \
		${DIR_SOURCE_KERNEL}/kernel.o \
		${DIR_SOURCE_KERNEL}/log.o \
		${DIR_SOURCE_KERNEL}/modules.o \
		${DIR_SOURCE_KERNEL}/vfs.o \
		${DIR_SOURCE_MODULES}/ata/ata.o \
		${DIR_SOURCE_MODULES}/elf/elf.o \
		${DIR_SOURCE_MODULES}/io/io.o \
		${DIR_SOURCE_MODULES}/io/ios.o \
		${DIR_SOURCE_MODULES}/kbd/kbd.o \
		${DIR_SOURCE_MODULES}/pci/pci.o \
		${DIR_SOURCE_MODULES}/pit/pit.o \
		${DIR_SOURCE_MODULES}/rtc/rtc.o \
		${DIR_SOURCE_MODULES}/rtl8139/rtl8139.o \
		${DIR_SOURCE_MODULES}/serial/serial.o \
		${DIR_SOURCE_MODULES}/tty/tty.o \
		${DIR_SOURCE_MODULES}/vga/vga.o \
		${DIR_SOURCE_ARCH}/arch.o \
		${DIR_SOURCE_ARCH}/calls.o \
		${DIR_SOURCE_ARCH}/cpu.o \
		${DIR_SOURCE_ARCH}/fpu.o \
		${DIR_SOURCE_ARCH}/fpus.o \
		${DIR_SOURCE_ARCH}/gdt.o \
		${DIR_SOURCE_ARCH}/gdts.o \
		${DIR_SOURCE_ARCH}/idt.o \
		${DIR_SOURCE_ARCH}/idts.o \
		${DIR_SOURCE_ARCH}/init.o \
		${DIR_SOURCE_ARCH}/irqs.o \
		${DIR_SOURCE_ARCH}/isrs.o \
		${DIR_SOURCE_ARCH}/irq.o \
		${DIR_SOURCE_ARCH}/isr.o \
		${DIR_SOURCE_ARCH}/mmu.o \
		${DIR_SOURCE_ARCH}/mboot.o \
		${DIR_SOURCE_ARCH}/syscall.o \
		${DIR_SOURCE_ARCH}/tss.o \
		${DIR_SOURCE_ARCH}/tsss.o \
		${DIR_SOURCE_LIB}/memory.o \
		${DIR_SOURCE_LIB}/file.o \
		${DIR_SOURCE_LIB}/string.o \
		-o ${DIR_IMAGE}/boot/kernel

ramdisk: kernel
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
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/cat.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/cat
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/cd.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/cd
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/clear.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/clear
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/cpu.o ${DIR_SOURCE_BIN}/cpus.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/cpu
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/date.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/date
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/echo.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/echo
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/elf.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/elf
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/hello.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/hello
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/ls.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/ls
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/reboot.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/reboot
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/shell.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/shell
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/tar.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/tar
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/timer.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/timer
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_BIN}/vga.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/vga
	@tar -cvf initrd.tar ${DIR_IMAGE}
	@find ${DIR_IMAGE} -depth -print | cpio -ov > initrd.cpio
	@mv initrd.tar ${DIR_IMAGE}/boot
	@mv initrd.cpio ${DIR_IMAGE}/boot

sda:
	@dd if=/dev/zero of=fudge.img bs=512 count=2880
	@dd if=${DIR_IMAGE}/boot/grub/stage1 conv=notrunc of=fudge.img bs=512 seek=0
	@dd if=${DIR_IMAGE}/boot/grub/stage2 conv=notrunc of=fudge.img bs=512 seek=1
	@dd if=menu.lst conv=notrunc of=fudge.img bs=512 seek=200
	@dd if=${DIR_IMAGE}/boot/kernel conv=notrunc of=fudge.img bs=512 seek=300
	@dd if=${DIR_IMAGE}/boot/initrd.tar conv=notrunc of=fudge.img bs=512 seek=400
	@sh x86-write-image.sh

iso:
	@genisoimage -R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table -o fudge.iso ${DIR_IMAGE}

hda:
	@dd if=/dev/zero of=hda.img bs=512 count=2880

clean:
	@rm -f fudge.img
	@rm -f fudge.iso
	@rm -f hda.img
	@rm -f ${DIR_IMAGE}/bin/cat
	@rm -f ${DIR_IMAGE}/bin/cd
	@rm -f ${DIR_IMAGE}/bin/clear
	@rm -f ${DIR_IMAGE}/bin/cpu
	@rm -f ${DIR_IMAGE}/bin/date
	@rm -f ${DIR_IMAGE}/bin/echo
	@rm -f ${DIR_IMAGE}/bin/elf
	@rm -f ${DIR_IMAGE}/bin/hello
	@rm -f ${DIR_IMAGE}/bin/ls
	@rm -f ${DIR_IMAGE}/bin/reboot
	@rm -f ${DIR_IMAGE}/bin/shell
	@rm -f ${DIR_IMAGE}/bin/tar
	@rm -f ${DIR_IMAGE}/bin/timer
	@rm -f ${DIR_IMAGE}/bin/vga
	@rm -f ${DIR_IMAGE}/boot/kernel
	@rm -f ${DIR_IMAGE}/boot/initrd
	@rm -f ${DIR_IMAGE}/boot/initrd.tar
	@rm -f ${DIR_IMAGE}/boot/initrd.cpio
	@rm -f ${DIR_SOURCE_BIN}/*.o
	@rm -f ${DIR_SOURCE_KERNEL}/*.o
	@rm -f ${DIR_SOURCE_KERNEL}/arch/arm/*.o
	@rm -f ${DIR_SOURCE_KERNEL}/arch/x86/*.o
	@rm -f ${DIR_SOURCE_LIB}/*.o
	@rm -f ${DIR_SOURCE_MODULES}/ata/*.o
	@rm -f ${DIR_SOURCE_MODULES}/elf/*.o
	@rm -f ${DIR_SOURCE_MODULES}/io/*.o
	@rm -f ${DIR_SOURCE_MODULES}/kbd/*.o
	@rm -f ${DIR_SOURCE_MODULES}/pci/*.o
	@rm -f ${DIR_SOURCE_MODULES}/pit/*.o
	@rm -f ${DIR_SOURCE_MODULES}/rtc/*.o
	@rm -f ${DIR_SOURCE_MODULES}/rtl8139/*.o
	@rm -f ${DIR_SOURCE_MODULES}/serial/*.o
	@rm -f ${DIR_SOURCE_MODULES}/tty/*.o
	@rm -f ${DIR_SOURCE_MODULES}/vga/*.o

