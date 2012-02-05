include rules.mk
include ${ARCH}.mk

.PHONY: all clean toolchain kernel lib modules user ramdisk

all: lib kernel kernel-${ARCH} modules user ramdisk

include lib/rules.mk
include kernel/rules.mk
include modules/rules.mk
include user/rules.mk

toolchain:
	@git submodule init toolchain
	@git submodule update toolchain
	@make -C toolchain all TARGET=${TARGET}

ramdisk:
	@cp kernel/fudge build/root/boot/fudge
	@nm build/root/boot/fudge | grep -f build/root/boot/fudge.sym > build/root/boot/fudge.map
	@cp modules/*/*.ko build/root/mod/
	@cp user/cat build/root/bin/cat
	@cp user/cd build/root/bin/cd
	@cp user/date build/root/bin/date
	@cp user/echo build/root/bin/echo
	@cp user/event1 build/root/bin/event1
	@cp user/event2 build/root/bin/event2
	@cp user/event3 build/root/bin/event3
	@cp user/hello build/root/bin/hello
	@cp user/init build/root/bin/init
	@cp user/load build/root/bin/load
	@cp user/ls build/root/bin/ls
	@cp user/reboot build/root/bin/reboot
	@cp user/shell build/root/bin/shell
	@cp user/tail build/root/bin/tail
	@cp user/timer build/root/bin/timer
	@cp user/unload build/root/bin/unload
	@cp user/test build/root/bin/test
	@tar -cvf initrd.tar build/root
	@find build/root -depth -print | cpio -ov > initrd.cpio
	@mv initrd.tar build/root/boot
	@mv initrd.cpio build/root/boot

image-arm:
	@arm-none-eabi-objcopy -O binary build/root/boot/fudge build/root/boot/fudge.bin
	@mkimage -A arm -C none -O linux -T kernel -d build/root/boot/fudge.bin -a 0x00100000 -e 0x00100000 build/root/boot/fudge.uimg
	@cat build/root/boot/uboot/u-boot.bin build/root/boot/fudge.uimg > fudge.img

sda:
	@dd if=/dev/zero of=fudge.img bs=512 count=2880
	@dd if=build/root/boot/grub/stage1 conv=notrunc of=fudge.img bs=512 seek=0
	@dd if=build/root/boot/grub/stage2 conv=notrunc of=fudge.img bs=512 seek=1
	@dd if=menu.lst conv=notrunc of=fudge.img bs=512 seek=200
	@dd if=build/root/boot/fudge conv=notrunc of=fudge.img bs=512 seek=300
	@dd if=build/root/boot/initrd.tar conv=notrunc of=fudge.img bs=512 seek=400
	@sh x86-write-image.sh

iso:
	@genisoimage -R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table -o fudge.iso build/root

hda:
	@dd if=/dev/zero of=hda.img bs=512 count=2880

clean: lib-clean kernel-clean modules-clean user-clean
	@rm -f fudge.img
	@rm -f fudge.iso
	@rm -f build/root/bin/cat
	@rm -f build/root/bin/cd
	@rm -f build/root/bin/date
	@rm -f build/root/bin/echo
	@rm -f build/root/bin/event1
	@rm -f build/root/bin/event2
	@rm -f build/root/bin/event3
	@rm -f build/root/bin/hello
	@rm -f build/root/bin/init
	@rm -f build/root/bin/load
	@rm -f build/root/bin/ls
	@rm -f build/root/bin/reboot
	@rm -f build/root/bin/shell
	@rm -f build/root/bin/tail
	@rm -f build/root/bin/timer
	@rm -f build/root/bin/unload
	@rm -f build/root/bin/test
	@rm -f build/root/mod/*.ko
	@rm -f build/root/boot/*.bin
	@rm -f build/root/boot/*.uimg
	@rm -f build/root/boot/fudge
	@rm -f build/root/boot/fudge.map
	@rm -f build/root/boot/initrd
	@rm -f build/root/boot/initrd.tar
	@rm -f build/root/boot/initrd.cpio

