ARCH=x86
DIR_IMAGE=build/root
DIR_SOURCE_USER=user
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

.PHONY: all clean kernel user ramdisk sda iso hda

all: ramdisk

kernel:
	@make -C lib/
	@make -C kernel/
	@make -C kernel/arch/x86/
	@make -C modules/
	@${LD} ${LDFLAGS} \
		${DIR_SOURCE_KERNEL}/error.o \
		${DIR_SOURCE_KERNEL}/event.o \
		${DIR_SOURCE_KERNEL}/initrd.o \
		${DIR_SOURCE_KERNEL}/kernel.o \
		${DIR_SOURCE_KERNEL}/log.o \
		${DIR_SOURCE_KERNEL}/modules.o \
		${DIR_SOURCE_KERNEL}/runtime.o \
		${DIR_SOURCE_KERNEL}/syscall.o \
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
		${DIR_SOURCE_MODULES}/stream/stream.o \
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

user:
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/cat.c -o ${DIR_SOURCE_USER}/cat.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/cd.c -o ${DIR_SOURCE_USER}/cd.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/clear.c -o ${DIR_SOURCE_USER}/clear.o
	@${ASM} ${ASMFLAGS} ${DIR_SOURCE_USER}/cpu.s -o ${DIR_SOURCE_USER}/cpus.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/cpu.c -o ${DIR_SOURCE_USER}/cpu.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/date.c -o ${DIR_SOURCE_USER}/date.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/echo.c -o ${DIR_SOURCE_USER}/echo.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/event1.c -o ${DIR_SOURCE_USER}/event1.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/event2.c -o ${DIR_SOURCE_USER}/event2.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/event3.c -o ${DIR_SOURCE_USER}/event3.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/hello.c -o ${DIR_SOURCE_USER}/hello.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/init.c -o ${DIR_SOURCE_USER}/init.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/ls.c -o ${DIR_SOURCE_USER}/ls.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/reboot.c -o ${DIR_SOURCE_USER}/reboot.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/shell.c -o ${DIR_SOURCE_USER}/shell.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/tar.c -o ${DIR_SOURCE_USER}/tar.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/timer.c -o ${DIR_SOURCE_USER}/timer.o
	@${GCC} ${GCCFLAGS_RAMDISK} ${DIR_SOURCE_USER}/vga.c -o ${DIR_SOURCE_USER}/vga.o
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/cat.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/cat
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/cd.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/cd
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/clear.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/clear
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/cpu.o ${DIR_SOURCE_USER}/cpus.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/cpu
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/date.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/date
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/echo.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/echo
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/event1.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/event1
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/event2.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/event2
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/event3.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/event3
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/hello.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/hello
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/init.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/init
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/ls.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/ls
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/reboot.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/reboot
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/shell.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/shell
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/tar.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/tar
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/timer.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/timer
	@${LD} ${LDFLAGS_RAMDISK} ${DIR_SOURCE_USER}/vga.o ${DIR_SOURCE_LIB}/memory.o ${DIR_SOURCE_LIB}/string.o ${DIR_SOURCE_LIB}/file.o ${DIR_SOURCE_ARCH}/calls.o -o ${DIR_IMAGE}/bin/vga

ramdisk: kernel user
	@cp ${DIR_SOURCE_MODULES}/test/test.o ${DIR_IMAGE}/lib/modules/test.ko
	@cp ${DIR_SOURCE_MODULES}/ata/ata.o ${DIR_IMAGE}/lib/modules/ata.ko
	@cp ${DIR_SOURCE_MODULES}/elf/elf.o ${DIR_IMAGE}/lib/modules/elf.ko
	@cp ${DIR_SOURCE_MODULES}/io/io.o ${DIR_IMAGE}/lib/modules/io.ko
	@cp ${DIR_SOURCE_MODULES}/io/ios.o ${DIR_IMAGE}/lib/modules/ios.ko
	@cp ${DIR_SOURCE_MODULES}/kbd/kbd.o ${DIR_IMAGE}/lib/modules/kbd.ko
	@cp ${DIR_SOURCE_MODULES}/pci/pci.o ${DIR_IMAGE}/lib/modules/pci.ko
	@cp ${DIR_SOURCE_MODULES}/pit/pit.o ${DIR_IMAGE}/lib/modules/pit.ko
	@cp ${DIR_SOURCE_MODULES}/rtc/rtc.o ${DIR_IMAGE}/lib/modules/rtc.ko
	@cp ${DIR_SOURCE_MODULES}/rtl8139/rtl8139.o ${DIR_IMAGE}/lib/modules/rtl8139.ko
	@cp ${DIR_SOURCE_MODULES}/serial/serial.o ${DIR_IMAGE}/lib/modules/serial.ko
	@cp ${DIR_SOURCE_MODULES}/tty/tty.o ${DIR_IMAGE}/lib/modules/tty.ko
	@cp ${DIR_SOURCE_MODULES}/vga/vga.o ${DIR_IMAGE}/lib/modules/vga.ko
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
	@make -C lib/ clean
	@make -C kernel/ clean
	@make -C kernel/arch/x86/ clean
	@make -C modules/ clean
	@rm -f fudge.img
	@rm -f fudge.iso
	@rm -f hda.img
	@rm -f ${DIR_IMAGE}/bin/cat
	@rm -f ${DIR_IMAGE}/bin/cd
	@rm -f ${DIR_IMAGE}/bin/clear
	@rm -f ${DIR_IMAGE}/bin/cpu
	@rm -f ${DIR_IMAGE}/bin/date
	@rm -f ${DIR_IMAGE}/bin/echo
	@rm -f ${DIR_IMAGE}/bin/event1
	@rm -f ${DIR_IMAGE}/bin/event2
	@rm -f ${DIR_IMAGE}/bin/event3
	@rm -f ${DIR_IMAGE}/bin/hello
	@rm -f ${DIR_IMAGE}/bin/init
	@rm -f ${DIR_IMAGE}/bin/ls
	@rm -f ${DIR_IMAGE}/bin/reboot
	@rm -f ${DIR_IMAGE}/bin/shell
	@rm -f ${DIR_IMAGE}/bin/tar
	@rm -f ${DIR_IMAGE}/bin/timer
	@rm -f ${DIR_IMAGE}/bin/vga
	@rm -f ${DIR_IMAGE}/lib/modules/*.ko
	@rm -f ${DIR_IMAGE}/boot/kernel
	@rm -f ${DIR_IMAGE}/boot/initrd
	@rm -f ${DIR_IMAGE}/boot/initrd.tar
	@rm -f ${DIR_IMAGE}/boot/initrd.cpio
	@rm -f ${DIR_SOURCE_USER}/*.o
	@rm -f ${DIR_SOURCE_KERNEL}/*.o

