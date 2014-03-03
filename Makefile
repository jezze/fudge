ARCH:=x86
LOADER:=mboot
PREFIX:=

INCLUDE_PATH:=include
LIBS_PATH:=libs
MODULES_PATH:=modules
MODULES_ARCH_PATH:=$(MODULES_PATH)/arch/$(ARCH)
PACKAGES_PATH:=packages
RAMDISK_PATH:=image
INSTALL_PATH:=/boot

KERNEL_NAME:=fudge
KERNEL:=$(KERNEL_NAME)

RAMDISK_NAME:=initrd
RAMDISK_TYPE:=tar
RAMDISK:=$(RAMDISK_NAME).$(RAMDISK_TYPE)

AS:=$(PREFIX)$(AS)
CC:=$(PREFIX)$(CC)
LD:=$(PREFIX)$(LD)

ASFLAGS_x86:=-32
CFLAGS_x86:=-m32
LDFLAGS_x86:=-melf_i386

ASFLAGS_arm:=-mfpu=softfpa
CFLAGS_arm:=
LDFLAGS_arm:=

ASFLAGS:=$(ASFLAGS_$(ARCH))
CFLAGS:=-Wall -Werror -ffreestanding -nostdlib -nostdinc -mno-red-zone -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH) $(CFLAGS_$(ARCH))
LDFLAGS:=$(LDFLAGS_$(ARCH))
ARFLAGS:=rs

LIBS:=
LIBS_OBJECTS:=
MODULES:=
MODULES_OBJECTS:=
PACKAGES:=
PACKAGES_OBJECTS:=
PACKAGES_CONFIGS:=
PACKAGES_SHARES:=

.PHONY: all clean install kernel libs modules packages ramdisk

all: libs modules packages kernel ramdisk

.s.o:
	$(AS) $(ASFLAGS) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

include $(LIBS_PATH)/rules.mk
include $(MODULES_PATH)/rules.mk
include $(PACKAGES_PATH)/rules.mk

$(KERNEL_NAME): $(LIBLOADER) $(LIBARCH) $(LIBKERNEL) $(LIBELF) $(LIBTAR) $(LIBCPIO) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -Tlibs/$(ARCH)/$(LOADER)/linker.ld -o $@ $^

$(RAMDISK_PATH): $(KERNEL) $(MODULES) $(PACKAGES) $(PACKAGES_CONFIGS) $(PACKAGES_SHARES)
	mkdir -p $@/bin
	mkdir -p $@/boot
	mkdir -p $@/boot/mod
	mkdir -p $@/config
	mkdir -p $@/home
	mkdir -p $@/kernel
	mkdir -p $@/share
	mkdir -p $@/system
	cp $(KERNEL) $@/boot
	cp $(MODULES) $@/boot/mod
	cp $(PACKAGES) $@/bin
	cp $(PACKAGES_CONFIGS) $@/config
	cp $(PACKAGES_SHARES) $@/share

$(RAMDISK_NAME).tar: $(RAMDISK_PATH)
	tar -cf $@ $(RAMDISK_PATH)

$(RAMDISK_NAME).cpio: $(RAMDISK_PATH)
	find $(RAMDISK_PATH) -depth | cpio -o > $@

clean:
	rm -rf $(KERNEL)
	rm -rf $(LIBS) $(LIBS_OBJECTS)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(PACKAGES) $(PACKAGES_OBJECTS)
	rm -rf $(RAMDISK_PATH) $(RAMDISK)

install:
	install -m 644 $(KERNEL) $(INSTALL_PATH)
	install -m 644 $(RAMDISK) $(INSTALL_PATH)

kernel: $(KERNEL_NAME)

libs: $(LIBS)

modules: $(MODULES)

packages: $(PACKAGES)

ramdisk: $(RAMDISK_NAME).$(RAMDISK_TYPE)
