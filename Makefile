ARCH:=x86
LOADER:=mboot

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

AS:=clang
CC:=clang
LD:=clang

ASFLAGS_x86:=-target i386-pc-none-elf -msoft-float
CFLAGS_x86:=-target i386-pc-none-elf -msoft-float
LDFLAGS_x86:=-target i386-pc-none-elf -msoft-float

ASFLAGS_arm:=-target arm-none-eabi -msoft-float
CFLAGS_arm:=-target arm-none-eabi -msoft-float
LDFLAGS_arm:=-target arm-none-eabi -msoft-float

ASFLAGS:=-ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH) $(ASFLAGS_$(ARCH))
CFLAGS:=-Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH) $(CFLAGS_$(ARCH))
LDFLAGS:=-Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH) $(LDFLAGS_$(ARCH))
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

all: kernel libs modules packages ramdisk

.s.o:
	$(AS) -c $(ASFLAGS) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

include $(LIBS_PATH)/rules.mk
include $(MODULES_PATH)/rules.mk
include $(PACKAGES_PATH)/rules.mk

$(KERNEL_NAME): $(LIBLOADER) $(LIBARCH) $(LIBKERNEL) $(LIBELF) $(LIBTAR) $(LIBCPIO) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -Tlibs/$(ARCH)/$(LOADER)/linker.ld -o $@ $^

$(RAMDISK_PATH): kernel libs modules packages
	mkdir -p $(RAMDISK_PATH)
	mkdir -p $(RAMDISK_PATH)/home
	mkdir -p $(RAMDISK_PATH)/kernel
	mkdir -p $(RAMDISK_PATH)/system

$(RAMDISK_NAME).tar: $(RAMDISK_PATH)
	tar -cf $@ $<

$(RAMDISK_NAME).cpio: $(RAMDISK_PATH)
	find $< -depth | cpio -o > $@

clean:
	rm -rf $(LIBS) $(LIBS_OBJECTS)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(PACKAGES) $(PACKAGES_OBJECTS)
	rm -rf $(KERNEL)
	rm -rf $(RAMDISK)
	rm -rf $(RAMDISK_PATH)

install:
	install -m 644 $(KERNEL) $(INSTALL_PATH)
	install -m 644 $(RAMDISK) $(INSTALL_PATH)

kernel: $(KERNEL)
	mkdir -p $(RAMDISK_PATH)/boot
	cp $(KERNEL) $(RAMDISK_PATH)/boot

libs: $(LIBS)
	mkdir -p $(RAMDISK_PATH)/lib
	cp $(LIBS) $(RAMDISK_PATH)/lib

modules: $(MODULES)
	mkdir -p $(RAMDISK_PATH)/boot/mod
	cp $(MODULES) $(RAMDISK_PATH)/boot/mod

packages: $(PACKAGES) $(PACKAGES_CONFIGS) $(PACKAGES_SHARES)
	mkdir -p $(RAMDISK_PATH)/bin
	mkdir -p $(RAMDISK_PATH)/config
	mkdir -p $(RAMDISK_PATH)/share
	cp $(PACKAGES) $(RAMDISK_PATH)/bin
	cp $(PACKAGES_CONFIGS) $(RAMDISK_PATH)/config
	cp $(PACKAGES_SHARES) $(RAMDISK_PATH)/share

ramdisk: $(RAMDISK)

