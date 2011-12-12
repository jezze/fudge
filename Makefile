ARCH=x86
DIR_IMAGE=build/root
DIR_SOURCE_KERNEL=kernel
DIR_SOURCE_LIB=lib
DIR_SOURCE_MODULES=modules
DIR_SOURCE_USER=user

.PHONY: all clean kernel lib modules user ramdisk sda iso hda

all: lib kernel modules user ramdisk-${ARCH}

kernel:
	@make -C ${DIR_SOURCE_KERNEL}/ all-${ARCH}

lib:
	@make -C ${DIR_SOURCE_LIB}/ all-${ARCH}

modules:
	@make -C ${DIR_SOURCE_MODULES}/ all-${ARCH}

user:
	@make -C ${DIR_SOURCE_USER}/ all-${ARCH}

ramdisk-arm:
	@cp ${DIR_SOURCE_KERNEL}/fudge ${DIR_IMAGE}/boot/fudge
	@nm ${DIR_IMAGE}/boot/fudge | grep -f ${DIR_IMAGE}/boot/fudge.sym > ${DIR_IMAGE}/boot/fudge.map
	@cp ${DIR_SOURCE_MODULES}/*/*.ko ${DIR_IMAGE}/mod/
	@cp ${DIR_SOURCE_USER}/cat ${DIR_IMAGE}/bin/cat
	@cp ${DIR_SOURCE_USER}/cd ${DIR_IMAGE}/bin/cd
	@cp ${DIR_SOURCE_USER}/clear ${DIR_IMAGE}/bin/clear
	@cp ${DIR_SOURCE_USER}/date ${DIR_IMAGE}/bin/date
	@cp ${DIR_SOURCE_USER}/echo ${DIR_IMAGE}/bin/echo
	@cp ${DIR_SOURCE_USER}/event1 ${DIR_IMAGE}/bin/event1
	@cp ${DIR_SOURCE_USER}/event2 ${DIR_IMAGE}/bin/event2
	@cp ${DIR_SOURCE_USER}/event3 ${DIR_IMAGE}/bin/event3
	@cp ${DIR_SOURCE_USER}/hello ${DIR_IMAGE}/bin/hello
	@cp ${DIR_SOURCE_USER}/init ${DIR_IMAGE}/bin/init
	@cp ${DIR_SOURCE_USER}/load ${DIR_IMAGE}/bin/load
	@cp ${DIR_SOURCE_USER}/ls ${DIR_IMAGE}/bin/ls
	@cp ${DIR_SOURCE_USER}/reboot ${DIR_IMAGE}/bin/reboot
	@cp ${DIR_SOURCE_USER}/shell ${DIR_IMAGE}/bin/shell
	@cp ${DIR_SOURCE_USER}/timer ${DIR_IMAGE}/bin/timer
	@cp ${DIR_SOURCE_USER}/unload ${DIR_IMAGE}/bin/unload
	@tar -cvf initrd.tar ${DIR_IMAGE}
	@find ${DIR_IMAGE} -depth -print | cpio -ov > initrd.cpio
	@mv initrd.tar ${DIR_IMAGE}/boot
	@mv initrd.cpio ${DIR_IMAGE}/boot

ramdisk-x86:
	@cp ${DIR_SOURCE_KERNEL}/fudge ${DIR_IMAGE}/boot/fudge
	@nm ${DIR_IMAGE}/boot/fudge | grep -f ${DIR_IMAGE}/boot/fudge.sym > ${DIR_IMAGE}/boot/fudge.map
	@cp ${DIR_SOURCE_MODULES}/*/*.ko ${DIR_IMAGE}/mod/
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
	@cp ${DIR_SOURCE_USER}/load ${DIR_IMAGE}/bin/load
	@cp ${DIR_SOURCE_USER}/ls ${DIR_IMAGE}/bin/ls
	@cp ${DIR_SOURCE_USER}/reboot ${DIR_IMAGE}/bin/reboot
	@cp ${DIR_SOURCE_USER}/shell ${DIR_IMAGE}/bin/shell
	@cp ${DIR_SOURCE_USER}/timer ${DIR_IMAGE}/bin/timer
	@cp ${DIR_SOURCE_USER}/unload ${DIR_IMAGE}/bin/unload
	@cp ${DIR_SOURCE_USER}/vga ${DIR_IMAGE}/bin/vga
	@tar -cvf initrd.tar ${DIR_IMAGE}
	@find ${DIR_IMAGE} -depth -print | cpio -ov > initrd.cpio
	@mv initrd.tar ${DIR_IMAGE}/boot
	@mv initrd.cpio ${DIR_IMAGE}/boot

image-arm:
	@arm-none-eabi-objcopy -O binary ${DIR_IMAGE}/boot/fudge ${DIR_IMAGE}/boot/fudge.bin
	@mkimage -A arm -C none -O linux -T kernel -d ${DIR_IMAGE}/boot/fudge.bin -a 0x00100000 -e 0x00100000 ${DIR_IMAGE}/boot/fudge.uimg
	@cat ${DIR_IMAGE}/boot/uboot/u-boot.bin ${DIR_IMAGE}/boot/fudge.uimg > fudge.img

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
	@make -C ${DIR_SOURCE_MODULES}/ clean
	@make -C ${DIR_SOURCE_USER}/ clean
	@rm -f fudge.img
	@rm -f fudge.iso
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
	@rm -f ${DIR_IMAGE}/bin/load
	@rm -f ${DIR_IMAGE}/bin/ls
	@rm -f ${DIR_IMAGE}/bin/reboot
	@rm -f ${DIR_IMAGE}/bin/shell
	@rm -f ${DIR_IMAGE}/bin/timer
	@rm -f ${DIR_IMAGE}/bin/unload
	@rm -f ${DIR_IMAGE}/bin/vga
	@rm -f ${DIR_IMAGE}/mod/*.ko
	@rm -f ${DIR_IMAGE}/boot/*.bin
	@rm -f ${DIR_IMAGE}/boot/*.uimg
	@rm -f ${DIR_IMAGE}/boot/fudge
	@rm -f ${DIR_IMAGE}/boot/fudge.map
	@rm -f ${DIR_IMAGE}/boot/initrd
	@rm -f ${DIR_IMAGE}/boot/initrd.tar
	@rm -f ${DIR_IMAGE}/boot/initrd.cpio

