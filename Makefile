ARCH:=x86
LOADER:=mboot
PREFIX:=

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
CFLAGS:=-Wall -Werror -ffreestanding -nostdlib -nostdinc -mno-red-zone -std=c89 -pedantic -O2 $(CFLAGS_$(ARCH))
LDFLAGS:=$(LDFLAGS_$(ARCH))
ARFLAGS:=rs

KERNEL_NAME:=fudge
KERNEL:=$(KERNEL_NAME)

RAMDISK_NAME:=initrd
RAMDISK_TYPE:=tar
RAMDISK:=$(RAMDISK_NAME).$(RAMDISK_TYPE)

LIBS_PATH:=libs
MODULES_PATH:=modules
MODULES_ARCH_PATH:=$(MODULES_PATH)/arch/$(ARCH)
PACKAGES_PATH:=packages
RAMDISK_PATH=image
INSTALL_PATH:=/boot

LIBS:=
LIBS_OBJECTS:=
MODULES:=
MODULES_OBJECTS:=
PACKAGES:=
PACKAGES_OBJECTS:=
PACKAGES_CONFIGS:=
PACKAGES_SHARES:=

.PHONY: all clean kernel libs modules packages

all: libs modules packages kernel ramdisk

.s.o:
	$(AS) $(ASFLAGS) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

include $(LIBS_PATH)/rules.mk
include $(MODULES_PATH)/rules.mk
include $(PACKAGES_PATH)/rules.mk

clean:
	rm -rf $(LIBS) $(LIBS_OBJECTS)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(PACKAGES) $(PACKAGES_OBJECTS)
	rm -rf $(KERNEL)
	rm -rf $(RAMDISK_PATH) $(RAMDISK)

$(RAMDISK_PATH)/bin: $(PACKAGES)
	mkdir -p $@
	cp $(PACKAGES) $@

$(RAMDISK_PATH)/boot:
	mkdir -p $@

$(RAMDISK_PATH)/boot/fudge: $(RAMDISK_PATH)/boot $(KERNEL)
	cp $(KERNEL) $@

$(RAMDISK_PATH)/boot/mod: $(RAMDISK_PATH)/boot $(MODULES)
	mkdir -p $@
	cp $(MODULES) $@

$(RAMDISK_PATH)/config: $(PACKAGES_CONFIGS)
	mkdir -p $@
	cp $(PACKAGES_CONFIGS) $@

$(RAMDISK_PATH)/share: $(PACKAGES_SHARES)
	mkdir -p $@
	cp $(PACKAGES_SHARES) $@

$(RAMDISK_PATH)/home:
	mkdir -p $@

$(RAMDISK_PATH)/system:
	mkdir -p $@

$(RAMDISK_PATH): $(RAMDISK_PATH)/bin $(RAMDISK_PATH)/boot $(RAMDISK_PATH)/boot/fudge $(RAMDISK_PATH)/boot/mod $(RAMDISK_PATH)/config $(RAMDISK_PATH)/home $(RAMDISK_PATH)/share $(RAMDISK_PATH)/system

install:
	install -m 644 $(KERNEL) $(INSTALL_PATH)
	install -m 644 $(RAMDISK) $(INSTALL_PATH)

kernel: $(KERNEL_NAME)

$(KERNEL_NAME): $(LIBLOADER) $(LIBARCH) $(LIBKERNEL) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -Tlibs/$(ARCH)/$(LOADER)/linker.ld -o $@ $^

libs: $(LIBS)

modules: $(MODULES)

packages: $(PACKAGES)

ramdisk: $(RAMDISK_NAME).$(RAMDISK_TYPE)

$(RAMDISK_NAME).tar: $(RAMDISK_PATH)
	tar -cf $@ $(RAMDISK_PATH)

$(RAMDISK_NAME).cpio: $(RAMDISK_PATH)
	find $(RAMDISK_PATH) -depth | cpio -o > $@
