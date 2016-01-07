ARCH:=x86
KERNEL:=fudge

DIR_SRC:=src
DIR_INCLUDE:=include
DIR_BUILD:=build
DIR_SNAPSHOT:=snapbuild
DIR_INSTALL:=/boot

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
CFLAGS:=-c -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(DIR_INCLUDE) -I$(DIR_SRC)
LDFLAGS:=

OBJ_ABI_x86:=$(DIR_SRC)/abi/x86/call.o $(DIR_SRC)/abi/x86/crt0.o
OBJ_ABI_arm:=$(DIR_SRC)/abi/arm/call.o $(DIR_SRC)/abi/arm/crt0.o
OBJ_ABI:=$(OBJ_ABI_$(ARCH))

OBJ_STD_x86:=
OBJ_STD_arm:=$(DIR_SRC)/std/arm/memcmp.o $(DIR_SRC)/std/arm/memcpy.o $(DIR_SRC)/std/arm/memmove.o $(DIR_SRC)/std/arm/memset.o $(DIR_SRC)/std/arm/setjmp.o $(DIR_SRC)/std/arm/strcmp.o $(DIR_SRC)/std/arm/strncmp.o $(DIR_SRC)/std/arm/gcc/__aeabi_idiv.o $(DIR_SRC)/std/arm/gcc/__aeabi_idivmod.o $(DIR_SRC)/std/arm/gcc/__aeabi_uidiv.o $(DIR_SRC)/std/arm/gcc/__aeabi_uidivmod.o $(DIR_SRC)/std/arm/gcc/__clzsi2.o $(DIR_SRC)/std/arm/gcc/__divsi3.o $(DIR_SRC)/std/arm/gcc/__modsi3.o $(DIR_SRC)/std/arm/gcc/__udivmodsi4.o $(DIR_SRC)/std/arm/gcc/__udivsi3.o $(DIR_SRC)/std/arm/gcc/__umodsi3.o
OBJ_STD:=$(OBJ_STD_$(ARCH))

RAMDISK_NAME:=$(KERNEL)
RAMDISK_TYPE:=cpio
RAMDISK:=$(RAMDISK_NAME).$(RAMDISK_TYPE)

IMAGE_NAME:=$(KERNEL)
IMAGE_TYPE:=img
IMAGE=$(IMAGE_NAME).$(IMAGE_TYPE)

.PHONY: all clean snapshot install

all: $(KERNEL) $(RAMDISK)

clean:
	rm -rf $(DIR_BUILD) $(KERNEL) $(RAMDISK) $(IMAGE) $(OBJ_ABI) $(OBJ_STD) $(OBJ) $(BIN) $(MOD)

snapshot: $(DIR_SNAPSHOT)

install: $(DIR_INSTALL)/$(KERNEL) $(DIR_INSTALL)/$(RAMDISK)

.s.o:
	$(AS) -o $@ $(ASFLAGS) $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $<

include $(DIR_SRC)/rules.mk

$(DIR_BUILD): $(BIN) $(MOD)
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

$(KERNEL): $(DIR_SRC)/kernel/$(KERNEL)
	cp $^ $@

$(RAMDISK_NAME).tar: $(DIR_BUILD)
	tar -cf $@ $^

$(RAMDISK_NAME).cpio: $(DIR_BUILD)
	find $^ -depth | cpio -o > $@

$(IMAGE): $(KERNEL) $(RAMDISK)
	dd if=/dev/zero of=$@ count=65536
	dd if=$(KERNEL) of=$@ conv=notrunc
	dd if=$(RAMDISK) of=$@ skip=4096 conv=notrunc

$(DIR_SNAPSHOT): $(KERNEL) $(RAMDISK)
	mkdir -p $@
	cp $^ $@
	echo `git describe --always` > $@/commit
	mv $@ `date --iso-8601=seconds`

$(DIR_INSTALL)/$(KERNEL): $(KERNEL)
	install -m 644 $^ $@

$(DIR_INSTALL)/$(RAMDISK): $(RAMDISK)
	install -m 644 $^ $@

