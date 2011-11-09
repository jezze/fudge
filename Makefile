ARCH=x86
DIR_IMAGE=build/root
DIR_SOURCE_USER=user
DIR_SOURCE_KERNEL=kernel
DIR_SOURCE_ARCH=kernel/arch/${ARCH}
DIR_SOURCE_LIB=lib
DIR_SOURCE_MODULES=modules

LD=ld
LDFLAGS=-T${DIR_SOURCE_ARCH}/linker.ld -melf_i386

.PHONY: all clean kernel user ramdisk sda iso hda

all: ramdisk

kernel:
	@make -C ${DIR_SOURCE_LIB}/
	@make -C ${DIR_SOURCE_KERNEL}/
	@make -C ${DIR_SOURCE_ARCH}/
	@make -C ${DIR_SOURCE_MODULES}/
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
		-o ${DIR_IMAGE}/boot/fudge
	@nm ${DIR_IMAGE}/boot/fudge | grep -f ${DIR_IMAGE}/boot/fudge.sym > ${DIR_IMAGE}/boot/fudge.map

user:
	@make -C ${DIR_SOURCE_USER}/

ramdisk: kernel user
	@cp ${DIR_SOURCE_MODULES}/ata/ata.o ${DIR_IMAGE}/lib/modules/ata.ko
	@cp ${DIR_SOURCE_MODULES}/elf/elf.o ${DIR_IMAGE}/lib/modules/elf.ko
	@cp ${DIR_SOURCE_MODULES}/io/io.o ${DIR_IMAGE}/lib/modules/io.ko
	@cp ${DIR_SOURCE_MODULES}/io/ios.o ${DIR_IMAGE}/lib/modules/ios.ko
	@cp ${DIR_SOURCE_MODULES}/kbd/kbd.o ${DIR_IMAGE}/lib/modules/kbd.ko
	@cp ${DIR_SOURCE_MODULES}/pci/pci.o ${DIR_IMAGE}/lib/modules/pci.ko
	@cp ${DIR_SOURCE_MODULES}/pit/pit.ko ${DIR_IMAGE}/lib/modules/pit.ko
	@cp ${DIR_SOURCE_MODULES}/rtc/rtc.ko ${DIR_IMAGE}/lib/modules/rtc.ko
	@cp ${DIR_SOURCE_MODULES}/rtl8139/rtl8139.ko ${DIR_IMAGE}/lib/modules/rtl8139.ko
	@cp ${DIR_SOURCE_MODULES}/serial/serial.o ${DIR_IMAGE}/lib/modules/serial.ko
	@cp ${DIR_SOURCE_MODULES}/tty/tty.o ${DIR_IMAGE}/lib/modules/tty.ko
	@cp ${DIR_SOURCE_MODULES}/vga/vga.o ${DIR_IMAGE}/lib/modules/vga.ko
	@cp ${DIR_SOURCE_USER}/cat ${DIR_IMAGE}/bin/cat
	@cp ${DIR_SOURCE_USER}/cd ${DIR_IMAGE}/bin/cd
	@cp ${DIR_SOURCE_USER}/clear ${DIR_IMAGE}/bin/clear
	@cp ${DIR_SOURCE_USER}/cpu ${DIR_IMAGE}/bin/cpu
	@cp ${DIR_SOURCE_USER}/date ${DIR_IMAGE}/bin/date
	@cp ${DIR_SOURCE_USER}/echo ${DIR_IMAGE}/bin/echo
	@cp ${DIR_SOURCE_USER}/event1 ${DIR_IMAGE}/bin/event1
	@cp ${DIR_SOURCE_USER}/event2 ${DIR_IMAGE}/bin/event2
	@cp ${DIR_SOURCE_USER}/event3 ${DIR_IMAGE}/bin/event3
	@cp ${DIR_SOURCE_USER}/hello ${DIR_IMAGE}/bin/hello
	@cp ${DIR_SOURCE_USER}/init ${DIR_IMAGE}/bin/init
	@cp ${DIR_SOURCE_USER}/ls ${DIR_IMAGE}/bin/ls
	@cp ${DIR_SOURCE_USER}/reboot ${DIR_IMAGE}/bin/reboot
	@cp ${DIR_SOURCE_USER}/shell ${DIR_IMAGE}/bin/shell
	@cp ${DIR_SOURCE_USER}/tar ${DIR_IMAGE}/bin/tar
	@cp ${DIR_SOURCE_USER}/timer ${DIR_IMAGE}/bin/timer
	@cp ${DIR_SOURCE_USER}/vga ${DIR_IMAGE}/bin/vga
	@tar -cvf initrd.tar ${DIR_IMAGE}
	@find ${DIR_IMAGE} -depth -print | cpio -ov > initrd.cpio
	@mv initrd.tar ${DIR_IMAGE}/boot
	@mv initrd.cpio ${DIR_IMAGE}/boot

sda:
	@dd if=/dev/zero of=fudge.img bs=512 count=2880
	@dd if=${DIR_IMAGE}/boot/grub/stage1 conv=notrunc of=fudge.img bs=512 seek=0
	@dd if=${DIR_IMAGE}/boot/grub/stage2 conv=notrunc of=fudge.img bs=512 seek=1
	@dd if=menu.lst conv=notrunc of=fudge.img bs=512 seek=200
	@dd if=${DIR_IMAGE}/boot/fudge conv=notrunc of=fudge.img bs=512 seek=300
	@dd if=${DIR_IMAGE}/boot/initrd.tar conv=notrunc of=fudge.img bs=512 seek=400
	@sh x86-write-image.sh

iso:
	@genisoimage -R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table -o fudge.iso ${DIR_IMAGE}

hda:
	@dd if=/dev/zero of=hda.img bs=512 count=2880

clean:
	@make -C ${DIR_SOURCE_LIB}/ clean
	@make -C ${DIR_SOURCE_KERNEL}/ clean
	@make -C ${DIR_SOURCE_ARCH}/ clean
	@make -C ${DIR_SOURCE_MODULES}/ clean
	@make -C ${DIR_SOURCE_USER}/ clean
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
	@rm -f ${DIR_IMAGE}/boot/fudge
	@rm -f ${DIR_IMAGE}/boot/fudge.map
	@rm -f ${DIR_IMAGE}/boot/initrd
	@rm -f ${DIR_IMAGE}/boot/initrd.tar
	@rm -f ${DIR_IMAGE}/boot/initrd.cpio

