include rules.mk
include rules.${ARCH}.mk

.PHONY: all clean toolchain kernel lib modules packages ramdisk

all: lib kernel modules packages ramdisk

include lib/rules.mk
include kernel/rules.mk
include modules/rules.mk
include packages/rules.mk

toolchain:
	@git submodule init toolchain
	@git submodule update toolchain
	@make -C toolchain all TARGET=${TARGET}

ramdisk:
	@cp kernel/fudge image/boot/fudge
	@nm image/boot/fudge | grep -f image/boot/fudge.sym > image/boot/fudge.map
	@tar -cvf initrd.tar image
	@find image -depth -print | cpio -ov > initrd.cpio
	@mv initrd.tar image/boot
	@mv initrd.cpio image/boot

image-arm:
	@arm-none-eabi-objcopy -O binary image/boot/fudge image/boot/fudge.bin
	@mkimage -A arm -C none -O linux -T kernel -d image/boot/fudge.bin -a 0x00100000 -e 0x00100000 image/boot/fudge.uimg
	@cat image/boot/uboot/u-boot.bin image/boot/fudge.uimg > fudge.img

sda:
	@dd if=/dev/zero of=fudge.img bs=512 count=2880
	@dd if=image/boot/grub/stage1 conv=notrunc of=fudge.img bs=512 seek=0
	@dd if=image/boot/grub/stage2 conv=notrunc of=fudge.img bs=512 seek=1
	@dd if=menu.lst conv=notrunc of=fudge.img bs=512 seek=200
	@dd if=image/boot/fudge conv=notrunc of=fudge.img bs=512 seek=300
	@dd if=image/boot/initrd.tar conv=notrunc of=fudge.img bs=512 seek=400
	@sh x86-write-image.sh

iso:
	@genisoimage -R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table -o fudge.iso image

hda:
	@dd if=/dev/zero of=hda.img bs=512 count=2880

clean: lib-clean kernel-clean modules-clean packages-clean
	@rm -f fudge.img
	@rm -f fudge.iso
	@rm -f image/boot/*.bin
	@rm -f image/boot/*.uimg
	@rm -f image/boot/fudge
	@rm -f image/boot/fudge.map
	@rm -f image/boot/initrd
	@rm -f image/boot/initrd.tar
	@rm -f image/boot/initrd.cpio

