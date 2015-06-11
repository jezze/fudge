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

BUILD_PATH:=build
INSTALL_PATH:=/boot
INCLUDE_PATH:=include
KERNEL:=fudge

PACKAGES_PATH:=packages
PACKAGES_CFLAGS:=-I$(INCLUDE_PATH) -I$(PACKAGES_PATH)
PACKAGES_DEPS_x86:=$(PACKAGES_PATH)/abi/x86/call.o $(PACKAGES_PATH)/abi/x86/crt0.o
PACKAGES_DEPS_arm:=$(PACKAGES_PATH)/abi/arm/call.o $(PACKAGES_PATH)/abi/arm/crt0.o
PACKAGES_DEPS_arm+=$(PACKAGES_PATH)/std/arm/memcmp.o $(PACKAGES_PATH)/std/arm/memcpy.o $(PACKAGES_PATH)/std/arm/memmove.o $(PACKAGES_PATH)/std/arm/memset.o $(PACKAGES_PATH)/std/arm/setjmp.o $(PACKAGES_PATH)/std/arm/strcmp.o $(PACKAGES_PATH)/std/arm/strncmp.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_idiv.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_idivmod.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_uidiv.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_uidivmod.o $(PACKAGES_PATH)/std/arm/gcc/__clzsi2.o $(PACKAGES_PATH)/std/arm/gcc/__divsi3.o $(PACKAGES_PATH)/std/arm/gcc/__modsi3.o $(PACKAGES_PATH)/std/arm/gcc/__udivmodsi4.o $(PACKAGES_PATH)/std/arm/gcc/__udivsi3.o $(PACKAGES_PATH)/std/arm/gcc/__umodsi3.o
PACKAGES_DEPS:=$(PACKAGES_PATH)/fudge/ascii.o $(PACKAGES_PATH)/fudge/buffer.o $(PACKAGES_PATH)/fudge/memory.o $(PACKAGES_PATH)/fudge/list.o
PACKAGES_DEPS+=$(PACKAGES_DEPS_$(ARCH))

MODULES_PATH:=modules
MODULES_CFLAGS:=-I$(INCLUDE_PATH) -I$(MODULES_PATH) -I$(PACKAGES_PATH)
MODULES_LDFLAGS:=-T$(MODULES_PATH)/linker.ld -r

RAMDISK_NAME:=$(KERNEL)
RAMDISK_TYPE:=cpio
RAMDISK:=$(RAMDISK_NAME).$(RAMDISK_TYPE)

IMAGE_NAME:=$(KERNEL)
IMAGE_TYPE:=img
IMAGE=$(IMAGE_NAME).$(IMAGE_TYPE)

ALL:=$(KERNEL) $(RAMDISK)
INSTALL:=$(INSTALL_PATH)/$(KERNEL) $(INSTALL_PATH)/$(RAMDISK)
CLEAN:=$(PACKAGES_DEPS) $(KERNEL) $(RAMDISK) $(BUILD_PATH) $(IMAGE)

.PHONY: all clean install

all: $(ALL)

install: $(INSTALL)

clean:
	rm -rf $(CLEAN)

.s.o:
	$(AS) -o $@ $(ASFLAGS) $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $<

$(PACKAGES_PATH)/%.o: CFLAGS+=$(PACKAGES_CFLAGS)
$(MODULES_PATH)/%.o: CFLAGS+=$(MODULES_CFLAGS)
$(MODULES_PATH)/%.ko: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.0: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.1: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.2: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.3: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.4: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.5: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.6: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.7: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.8: LDFLAGS+=$(MODULES_LDFLAGS)
$(MODULES_PATH)/%.ko.9: LDFLAGS+=$(MODULES_LDFLAGS)

DIR:=$(MODULES_PATH)
include $(DIR)/rules.mk
DIR:=$(PACKAGES_PATH)
include $(DIR)/rules.mk

$(BUILD_PATH): $(BINS) $(MODULES)
	mkdir -p $@
	mkdir -p $@/bin
	cp $(BINS) $@/bin
	mkdir -p $@/mod
	cp $(MODULES) $@/mod
	mkdir -p $@/config
	cp config/* $@/config
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
	cp share/* $@/share
	mkdir -p $@/system

$(KERNEL): $(PACKAGES_PATH)/kernel/$(KERNEL)
	cp $< $@

$(RAMDISK_NAME).tar: $(BUILD_PATH)
	tar -cf $@ $^

$(RAMDISK_NAME).cpio: $(BUILD_PATH)
	find $^ -depth | cpio -o > $@

$(IMAGE): $(KERNEL) $(RAMDISK)
	dd if=/dev/zero of=$@ count=65536
	dd if=$(KERNEL) of=$@ conv=notrunc
	dd if=$(RAMDISK) of=$@ skip=4096 conv=notrunc

$(INSTALL_PATH)/$(KERNEL): $(KERNEL)
	install -m 644 $^ $@

$(INSTALL_PATH)/$(RAMDISK): $(RAMDISK)
	install -m 644 $^ $@

