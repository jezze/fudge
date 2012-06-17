ARCH=x86
TARGET=
AS=$(TARGET)as
CC=$(TARGET)gcc
CCFLAGS=-Wall -ffreestanding -nostdlib -std=c89 -pedantic
LD=$(TARGET)ld
AR=ar
ARFLAGS=rs

.PHONY: all lib kernel modules packages

all: lib/libfudge.a kernel/fudge modules packages ramdisk fudge.iso

include rules.$(ARCH).mk
include lib/rules.mk
include kernel/rules.mk
include modules/rules.mk
include packages/rules.mk

%.o: %.s
	$(AS) -c $(ASFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CCFLAGS) -o $@ $<

%: %.c lib/libfudge.a
	$(CC) -s $(CCFLAGS) -o $@ $< lib/libfudge.a

clean:
	rm -rf $(CLEAN)
	rm -f fudge.iso
	rm -rf image/bin
	rm -rf image/mod
	rm -f image/boot/fudge
	rm -f image/boot/initrd.tar
	rm -f image/boot/initrd.cpio

ramdisk:
	mkdir -p image/bin
	cp $(PACKAGES) image/bin
	mkdir -p image/mod
	cp $(MODULES) image/mod
	cp kernel/fudge image/boot/fudge
	tar -cf initrd.tar image
	find image -depth | cpio -o > initrd.cpio
	mv initrd.tar image/boot
	mv initrd.cpio image/boot

fudge.iso:
	genisoimage -R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table -o $@ image

toolchain:
	git submodule init toolchain
	git submodule update toolchain
	make -C toolchain all TARGET=${TARGET}

image-arm:
	arm-none-eabi-objcopy -O binary image/boot/fudge image/boot/fudge.bin
	mkimage -A arm -C none -O linux -T kernel -d image/boot/fudge.bin -a 0x00100000 -e 0x00100000 image/boot/fudge.uimg
	cat image/boot/uboot/u-boot.bin image/boot/fudge.uimg > fudge.img

sda:
	dd if=/dev/zero of=fudge.img bs=512 count=2880
	dd if=image/boot/grub/stage1 conv=notrunc of=fudge.img bs=512 seek=0
	dd if=image/boot/grub/stage2 conv=notrunc of=fudge.img bs=512 seek=1
	dd if=menu.lst conv=notrunc of=fudge.img bs=512 seek=200
	dd if=image/boot/fudge conv=notrunc of=fudge.img bs=512 seek=300
	dd if=image/boot/initrd.tar conv=notrunc of=fudge.img bs=512 seek=400
	sh x86-write-image.sh


