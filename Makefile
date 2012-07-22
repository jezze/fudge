ARCH=x86
PREFIX=
AS=$(PREFIX)as
CC=$(PREFIX)gcc
CCFLAGS=-Wall -Werror -ffreestanding -nostdlib -std=c89 -pedantic -O2
LD=$(PREFIX)ld
AR=ar
ARFLAGS=rs
IMAGE=image image/bin image/boot image/home image/data image/mod image/boot/fudge image/boot/initrd.tar

.PHONY: all lib kernel modules packages image

all: fudge.iso

include rules.$(ARCH).mk
include lib/rules.mk
include kernel/rules.mk
include modules/rules.mk
include packages/rules.mk

clean:
	rm -rf $(LIB) $(LIB_OBJECTS)
	rm -rf $(KERNEL) $(KERNEL_OBJECTS)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(PACKAGES)
	rm -rf $(IMAGE)
	rm -rf fudge.iso

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

image:
	mkdir -p $@

image/bin: image $(PACKAGES)
	mkdir -p $@
	cp $(PACKAGES) $@

image/boot: image
	cp -r system/boot $@

image/boot/fudge: image/boot $(KERNEL)
	cp $(KERNEL) $@

image/boot/initrd.tar: image/boot initrd.tar
	mv initrd.tar $@

image/boot/initrd.cpio: image/boot initrd.cpio
	mv initrd.cpio $@

image/home: image
	cp -r system/home $@

image/data: image
	cp -r system/data $@

image/mod: image $(MODULES)
	mkdir -p $@
	cp $(MODULES) $@

initrd.tar: image
	tar -cf $@ image

initrd.cpio: image
	find image -depth | cpio -o > $@

kernel: $(KERNEL)

lib: $(LIB)

modules: $(MODULES)

packages: $(PACKAGES)

%.o: %.s
	$(AS) -c $(ASFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CCFLAGS) -o $@ $<

%: %.c $(LIB)
	$(CC) -s $(CCFLAGS) -o $@ $< $(LIB)

# Experimental

image-arm: image/boot/fudge
	arm-none-eabi-objcopy -O binary image/boot/fudge image/boot/fudge.bin
	mkimage -A arm -C none -O linux -T kernel -d image/boot/fudge.bin -a 0x00100000 -e 0x00100000 image/boot/fudge.uimg
	cat image/boot/uboot/u-boot.bin image/boot/fudge.uimg > fudge.img

toolchain:
	git submodule init toolchain
	git submodule update toolchain
	make -C toolchain all PREFIX=$(PREFIX)
