ARCH=x86
PREFIX=
AS=$(PREFIX)as
CC=$(PREFIX)gcc
CFLAGS=-Wall -Werror -ffreestanding -nostdlib -std=c89 -pedantic -O2
LD=$(PREFIX)ld
AR=ar
ARFLAGS=rs

KERNEL_NAME=fudge
KERNEL=$(KERNEL_NAME)

RAMDISK_NAME=initrd
RAMDISK_TYPE=tar
RAMDISK=$(RAMDISK_NAME).$(RAMDISK_TYPE)

INSTALL_PATH=/boot

LIBS:=
LIBS_OBJECTS:=
MODULES:=
MODULES_OBJECTS:=
PACKAGES:=
PACKAGES_OBJECTS:=

.PHONY: all clean kernel libs modules packages

all: libs modules packages kernel ramdisk

include rules.$(ARCH).mk
include libs/rules.mk
include modules/rules.mk
include packages/rules.mk

clean:
	rm -rf $(LIBS) $(LIBS_OBJECTS)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(PACKAGES) $(PACKAGES_OBJECTS)
	rm -rf $(KERNEL)
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

kernel: $(KERNEL_NAME).$(ARCH)

$(KERNEL_NAME).arm: $(LIBVERSATILEPB) $(LIBKERNEL) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -Tlibs/versatilepb/linker.ld -o $(KERNEL_NAME) $^

$(KERNEL_NAME).x86: $(LIBMBOOT) $(LIBKERNEL) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -Tlibs/mboot/linker.ld -o $(KERNEL_NAME) $^

libs: $(LIBS)

modules: $(MODULES)

packages: $(PACKAGES)

ramdisk: $(RAMDISK_NAME).$(RAMDISK_TYPE)

$(RAMDISK_NAME).tar: image/bin image/boot image/boot/fudge image/home image/data image/mod
	tar -cf $@ image
	rm -rf image

$(RAMDISK_NAME).cpio: image/bin image/boot image/boot/fudge image/home image/data image/mod
	find image -depth | cpio -o > $@
	rm -rf image
