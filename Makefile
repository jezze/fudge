ARCH:=x86
KERNEL:=fudge

TARGET_x86:=i386-unknown-elf
TARGET_arm:=arm-unknown-eabi
TARGET:=$(TARGET_$(ARCH))

PLATFORM_x86:=pc
PLATFORM_arm:=integratorcp
PLATFORM:=$(PLATFORM_$(ARCH))

SRC_PATH:=src
INCLUDE_PATH:=include
BUILD_PATH:=build
INSTALL_PATH:=/boot

AS:=$(TARGET)-as
CC:=$(TARGET)-cc
LD:=$(TARGET)-ld

ASFLAGS:=
CFLAGS:=-c -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(SRC_PATH)
LDFLAGS:=

OBJ_ABI_x86:=$(SRC_PATH)/abi/x86/call.o $(SRC_PATH)/abi/x86/crt0.o
OBJ_ABI_arm:=$(SRC_PATH)/abi/arm/call.o $(SRC_PATH)/abi/arm/crt0.o
OBJ_STD_arm:=$(SRC_PATH)/std/arm/memcmp.o $(SRC_PATH)/std/arm/memcpy.o $(SRC_PATH)/std/arm/memmove.o $(SRC_PATH)/std/arm/memset.o $(SRC_PATH)/std/arm/setjmp.o $(SRC_PATH)/std/arm/strcmp.o $(SRC_PATH)/std/arm/strncmp.o $(SRC_PATH)/std/arm/gcc/__aeabi_idiv.o $(SRC_PATH)/std/arm/gcc/__aeabi_idivmod.o $(SRC_PATH)/std/arm/gcc/__aeabi_uidiv.o $(SRC_PATH)/std/arm/gcc/__aeabi_uidivmod.o $(SRC_PATH)/std/arm/gcc/__clzsi2.o $(SRC_PATH)/std/arm/gcc/__divsi3.o $(SRC_PATH)/std/arm/gcc/__modsi3.o $(SRC_PATH)/std/arm/gcc/__udivmodsi4.o $(SRC_PATH)/std/arm/gcc/__udivsi3.o $(SRC_PATH)/std/arm/gcc/__umodsi3.o
BIN_DEPS:=$(OBJ_ABI_$(ARCH)) $(OBJ_STD_$(ARCH))

RAMDISK_NAME:=$(KERNEL)
RAMDISK_TYPE:=cpio
RAMDISK:=$(RAMDISK_NAME).$(RAMDISK_TYPE)

IMAGE_NAME:=$(KERNEL)
IMAGE_TYPE:=img
IMAGE=$(IMAGE_NAME).$(IMAGE_TYPE)

ALL:=$(KERNEL) $(RAMDISK)
INSTALL:=$(INSTALL_PATH)/$(KERNEL) $(INSTALL_PATH)/$(RAMDISK)
CLEAN:=$(BIN) $(BIN_DEPS) $(KERNEL) $(KERNEL_DEPS) $(RAMDISK) $(BUILD_PATH) $(IMAGE)

.PHONY: all clean install

all: $(ALL)

install: $(INSTALL)

clean:
	rm -rf $(CLEAN)

.s.o:
	$(AS) -o $@ $(ASFLAGS) $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $<

include $(SRC_PATH)/rules.mk

$(BUILD_PATH): $(BIN) $(MOD)
	mkdir -p $@
	mkdir -p $@/bin
	cp $(BIN) $@/bin
	mkdir -p $@/mod
	cp $(MOD) $@/mod
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

$(KERNEL): $(SRC_PATH)/kernel/$(KERNEL)
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

