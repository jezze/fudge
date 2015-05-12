ARCH:=x86

TARGET_x86:=i386-unknown-elf
TARGET_arm:=arm-unknown-eabi
TARGET:=$(TARGET_$(ARCH))

PLATFORM_x86:=pc
PLATFORM_arm:=integratorcp
PLATFORM:=$(PLATFORM_$(ARCH))

AS:=$(TARGET)-as
CC:=$(TARGET)-cc
LD:=$(TARGET)-ld

ASFLAGS:=
CFLAGS:=-c -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2
LDFLAGS:=

BUILD_ROOT:=build
BUILD_BIN:=$(BUILD_ROOT)/bin
BUILD_BOOT:=$(BUILD_ROOT)/boot
BUILD_MODULE:=$(BUILD_BOOT)/mod

INSTALL_PATH:=/boot

INCLUDE_PATH:=include

LIBS_PATH:=libs
LIBS_CFLAGS:=-I$(INCLUDE_PATH) -I$(LIBS_PATH)

KERNEL:=fudge
KERNEL_LDFLAGS:=-Tplatform/$(PLATFORM)/linker.ld
KERNEL_DEPS_x86:=$(LIBS_PATH)/mboot/libmboot.a $(LIBS_PATH)/arch/$(ARCH)/libarch.a $(LIBS_PATH)/kernel/libkernel.a $(LIBS_PATH)/elf/libelf.a $(LIBS_PATH)/tar/libtar.a $(LIBS_PATH)/cpio/libcpio.a $(LIBS_PATH)/fudge/libfudge.a
KERNEL_DEPS_arm:=$(LIBS_PATH)/arch/$(ARCH)/libarch.a $(LIBS_PATH)/kernel/libkernel.a $(LIBS_PATH)/elf/libelf.a $(LIBS_PATH)/tar/libtar.a $(LIBS_PATH)/cpio/libcpio.a $(LIBS_PATH)/fudge/libfudge.a $(LIBS_PATH)/std/$(ARCH)/libstd.a
KERNEL_DEPS:=$(KERNEL_DEPS_$(ARCH))

MODULES_PATH:=modules
MODULES_CFLAGS:=-I$(INCLUDE_PATH) -I$(LIBS_PATH) -I$(MODULES_PATH)
MODULES_LDFLAGS:=-T$(MODULES_PATH)/linker.ld -r

PACKAGES_PATH:=packages
PACKAGES_CFLAGS:=-I$(INCLUDE_PATH) -I$(LIBS_PATH) -I$(PACKAGES_PATH)
PACKAGES_DEPS_x86:=$(LIBS_PATH)/abi/$(ARCH)/libabi.a $(LIBS_PATH)/fudge/libfudge.a
PACKAGES_DEPS_arm:=$(LIBS_PATH)/abi/$(ARCH)/libabi.a $(LIBS_PATH)/fudge/libfudge.a $(LIBS_PATH)/std/$(ARCH)/libstd.a
PACKAGES_DEPS:=$(PACKAGES_DEPS_$(ARCH))

RAMDISK_NAME:=initrd
RAMDISK_TYPE:=tar
RAMDISK:=$(RAMDISK_NAME).$(RAMDISK_TYPE)

IMAGE_NAME:=$(KERNEL)
IMAGE_TYPE:=img
IMAGE=$(IMAGE_NAME).$(IMAGE_TYPE)

ALL:=$(LIBS_PATH) $(MODULES_PATH) $(PACKAGES_PATH) $(BUILD_BOOT)/$(KERNEL) $(RAMDISK)
CLEAN:=$(BUILD_ROOT) $(KERNEL) $(RAMDISK) $(IMAGE)

.PHONY: all clean install

all: $(ALL)

.s.o:
	$(AS) -o $@ $(ASFLAGS) $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $<

$(LIBS_PATH)/%.o: CFLAGS+=$(LIBS_CFLAGS)
$(MODULES_PATH)/%.o: CFLAGS+=$(MODULES_CFLAGS)
$(PACKAGES_PATH)/%.o: CFLAGS+=$(PACKAGES_CFLAGS)

$(KERNEL): LDFLAGS+=$(KERNEL_LDFLAGS)
$(BUILD_MODULE)/%.ko: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.0: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.1: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.2: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.3: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.4: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.5: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.6: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.7: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.8: LDFLAGS+=$(MODULES_LDFLAGS)
$(BUILD_MODULE)/%.ko.9: LDFLAGS+=$(MODULES_LDFLAGS)

DIR:=$(LIBS_PATH)
include $(DIR)/rules.mk
DIR:=$(MODULES_PATH)
include $(DIR)/rules.mk
DIR:=$(PACKAGES_PATH)
include $(DIR)/rules.mk

$(BUILD_ROOT):
	mkdir -p $@
	mkdir -p $@/bin
	mkdir -p $@/boot/mod
	mkdir -p $@/config
	mkdir -p $@/home
	mkdir -p $@/mount
	mkdir -p $@/mount/0
	mkdir -p $@/mount/1
	mkdir -p $@/mount/2
	mkdir -p $@/mount/3
	mkdir -p $@/mount/4
	mkdir -p $@/mount/5
	mkdir -p $@/mount/6
	mkdir -p $@/mount/7
	mkdir -p $@/share
	mkdir -p $@/system
	cp config/* $@/config
	cp share/* $@/share

$(BUILD_BOOT): $(BUILD_ROOT)
	mkdir -p $@

$(BUILD_BOOT)/$(KERNEL): $(BUILD_BOOT) $(KERNEL)
	cp $(KERNEL) $@

$(KERNEL): $(KERNEL_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

$(RAMDISK_NAME).tar: $(BUILD_ROOT)
	tar -cf $@ $^

$(RAMDISK_NAME).cpio: $(BUILD_ROOT)
	find $^ -depth | cpio -o > $@

$(IMAGE): $(KERNEL) $(RAMDISK)
	dd if=/dev/zero of=$@ count=65536
	dd if=$(KERNEL) of=$@ conv=notrunc
	dd if=$(RAMDISK) of=$@ skip=4096 conv=notrunc

$(INSTALL_PATH)/$(KERNEL): $(KERNEL)
	install -m 644 $^ $@

$(INSTALL_PATH)/$(RAMDISK): $(RAMDISK)
	install -m 644 $^ $@

$(LIBS_PATH): $(BUILD_ROOT) $(LIBS)

$(MODULES_PATH): $(BUILD_ROOT) $(LIBS) $(MODULES)

$(PACKAGES_PATH): $(BUILD_ROOT) $(LIBS) $(BINS)

clean:
	rm -rf $(CLEAN)

install: $(INSTALL_PATH)/$(KERNEL) $(INSTALL_PATH)/$(RAMDISK)
