ARCH=x86
PREFIX=
AS=$(PREFIX)as
CC=$(PREFIX)gcc
CFLAGS=-Wall -Werror -ffreestanding -nostdlib -std=c89 -pedantic -O2
LD=$(PREFIX)ld
AR=ar
ARFLAGS=rs
IMAGE=image image/bin image/boot image/home image/data image/mod image/boot/fudge image/boot/initrd.tar

.PHONY: all clean lib kernel modules packages

all: fudge

include rules.$(ARCH).mk
include lib/rules.$(ARCH).mk
include lib/rules.mk
include kernel/rules.$(ARCH).mk
include kernel/rules.mk
include modules/rules.$(ARCH).mk
include modules/rules.mk
include packages/rules.mk

clean:
	rm -rf $(LIB) $(LIB_OBJECTS)
	rm -rf $(KERNEL) $(KERNEL_OBJECTS)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(PACKAGES)
	rm -rf $(IMAGE)

fudge: $(LIB) $(KERNEL) $(MODULES) $(PACKAGES) $(IMAGE)

image:
	mkdir -p $@

image/bin: image $(PACKAGES)
	mkdir -p $@
	cp $(PACKAGES) $@

image/boot: image
	mkdir -p $@

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
