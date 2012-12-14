ARCH=x86
PREFIX=
AS=$(PREFIX)as
CC=$(PREFIX)gcc
CFLAGS=-Wall -Werror -ffreestanding -nostdlib -std=c89 -pedantic -O2
LD=$(PREFIX)ld
AR=ar
ARFLAGS=rs

RAMDISK_NAME=initrd
RAMDISK_TYPE=tar
RAMDISK=$(RAMDISK_NAME).$(RAMDISK_TYPE)

INSTALL_PATH=/boot

.PHONY: all clean libs kernel modules packages

default: all

include rules.$(ARCH).mk
include libs/rules.mk
include kernel/rules.mk
include modules/rules.mk
include packages/rules.mk

all: $(KERNEL) $(LIBS) $(MODULES) $(PACKAGES) $(RAMDISK)

clean:
	rm -rf $(KERNEL) $(KERNEL_OBJECTS)
	rm -rf $(LIBS) $(LIBS_OBJECTS)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(PACKAGES)
	rm -rf $(RAMDISK)

image/bin: $(PACKAGES)
	mkdir -p $@
	cp $(PACKAGES) $@

image/boot:
	mkdir -p $@

image/boot/fudge: image/boot $(KERNEL)
	cp $(KERNEL) $@

image/home: system/home
	cp -r $< $@

image/data: system/data
	cp -r $< $@

image/mod: $(MODULES)
	mkdir -p $@
	cp $(MODULES) $@

install:
	install -m 644 $(KERNEL) $(INSTALL_PATH)
	install -m 644 $(RAMDISK) $(INSTALL_PATH)

kernel: $(KERNEL)

libs: $(LIBS)

modules: $(MODULES)

packages: $(PACKAGES)

$(RAMDISK_NAME).tar: image/bin image/boot image/boot/fudge image/home image/data image/mod
	tar -cf $@ image
	rm -rf image

$(RAMDISK_NAME).cpio: image/bin image/boot image/boot/fudge image/home image/data image/mod
	find image -depth | cpio -o > $@
	rm -rf image
