ARCH=x86
PREFIX=
AS=$(PREFIX)as
CC=$(PREFIX)gcc
CCFLAGS=-Wall -Werror -ffreestanding -nostdlib -std=c89 -pedantic -O2
LD=$(PREFIX)ld
AR=ar
ARFLAGS=rs

.PHONY: all lib kernel modules packages image

all: fudge.iso

include rules.$(ARCH).mk
include lib/rules.mk
include kernel/rules.mk
include modules/rules.mk
include packages/rules.mk

lib: $(LIB)

kernel: $(KERNEL)

modules: $(MODULES)

packages: $(PACKAGES)

%.o: %.s
	$(AS) -c $(ASFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CCFLAGS) -o $@ $<

%: %.c $(LIB)
	$(CC) -s $(CCFLAGS) -o $@ $< $(LIB)

IMAGE=image image/bin image/mod image/boot/fudge image/boot/initrd.tar

clean:
	rm -rf $(LIB) $(LIB_OBJECTS)
	rm -rf $(KERNEL) $(KERNEL_OBJECTS)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(PACKAGES)
	rm -rf $(IMAGE)
	rm -rf fudge.iso

image:
	mkdir -p $@
	cp -r system/* $@

image/bin: $(PACKAGES)
	mkdir -p $@
	cp $(PACKAGES) $@

image/mod: $(MODULES)
	mkdir -p $@
	cp $(MODULES) $@

image/boot/fudge: $(KERNEL)
	cp $(KERNEL) $@

image/boot/initrd.tar: initrd.tar
	mv initrd.tar $@

image/boot/initrd.cpio: initrd.cpio
	mv initrd.cpio $@

fudge.iso: $(LIB) $(KERNEL) $(MODULES) $(PACKAGES) $(IMAGE)
	genisoimage -R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table -o $@ image

fudge.img: $(LIB) $(KERNEL) $(MODULES) $(PACKAGES) $(IMAGE)
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=image/boot/grub/stage1 conv=notrunc of=$@ bs=512 seek=0
	dd if=image/boot/grub/stage2 conv=notrunc of=$@ bs=512 seek=1
	dd if=menu.lst conv=notrunc of=$@ bs=512 seek=200
	dd if=image/boot/fudge conv=notrunc of=$@ bs=512 seek=300
	dd if=image/boot/initrd.tar conv=notrunc of=$@ bs=512 seek=400
	sh x86-write-image.sh

initrd.tar:
	tar -cf $@ image

initrd.cpio:
	find image -depth | cpio -o > $@

# Experimental

image-arm: image/boot/fudge
	arm-none-eabi-objcopy -O binary image/boot/fudge image/boot/fudge.bin
	mkimage -A arm -C none -O linux -T kernel -d image/boot/fudge.bin -a 0x00100000 -e 0x00100000 image/boot/fudge.uimg
	cat image/boot/uboot/u-boot.bin image/boot/fudge.uimg > fudge.img

toolchain:
	git submodule init toolchain
	git submodule update toolchain
	make -C toolchain all PREFIX=$(PREFIX)
