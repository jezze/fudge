ARCH:=x86
LOADER:=mboot

INSTALL_PATH:=/boot
INCLUDE_PATH:=include
BUILD_PATH:=build
LIBS_PATH:=libs
MODULES_PATH:=modules
MODULES_ARCH_PATH:=$(MODULES_PATH)/arch/$(ARCH)
PACKAGES_PATH:=packages

MODULES:=
MODULES_OBJECTS:=

BINS:=
CONFS:=
LIBS:=
SHARES:=
CLEAN:=

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

ASFLAGS:=-c -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH) $(ASFLAGS_$(ARCH))
CFLAGS:=-c -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH) $(CFLAGS_$(ARCH))
LDFLAGS:=-Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH) $(LDFLAGS_$(ARCH))
ARFLAGS:=rs

.PHONY: all clean install kernel libs modules packages ramdisk

all: kernel libs modules packages ramdisk

.s.o:
	$(AS) -o $@ $(ASFLAGS) $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $<

DIR:=$(LIBS_PATH)
include $(DIR)/rules.mk
DIR:=$(MODULES_PATH)
include $(DIR)/rules.mk
DIR:=$(PACKAGES_PATH)
include $(DIR)/rules.mk

$(KERNEL_NAME): libs
	$(LD) -o $@ $(LDFLAGS) -Tlibs/$(ARCH)/$(LOADER)/linker.ld -L$(BUILD_PATH)/lib -static -lmboot -larch -lkernel -lelf -ltar -lcpio -lfudge

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)
	mkdir -p $(BUILD_PATH)/bin
	mkdir -p $(BUILD_PATH)/boot
	mkdir -p $(BUILD_PATH)/boot/mod
	mkdir -p $(BUILD_PATH)/config
	mkdir -p $(BUILD_PATH)/home
	mkdir -p $(BUILD_PATH)/kernel
	mkdir -p $(BUILD_PATH)/lib
	mkdir -p $(BUILD_PATH)/share
	mkdir -p $(BUILD_PATH)/system

$(RAMDISK_NAME).tar: $(BUILD_PATH)
	tar -cf $@ $^

$(RAMDISK_NAME).cpio: $(BUILD_PATH)
	find $^ -depth | cpio -o > $@

clean:
	rm -rf $(BUILD_PATH)
	rm -rf $(CLEAN)
	rm -rf $(MODULES) $(MODULES_OBJECTS)
	rm -rf $(KERNEL)
	rm -rf $(RAMDISK)

install: $(KERNEL) $(RAMDISK)
	install -m 644 $(KERNEL) $(INSTALL_PATH)
	install -m 644 $(RAMDISK) $(INSTALL_PATH)

kernel: $(BUILD_PATH) $(KERNEL)
	cp $(KERNEL) $(BUILD_PATH)/boot

libs: $(BUILD_PATH) $(LIBS)
	cp $(LIBS) $(BUILD_PATH)/lib

modules: $(BUILD_PATH) $(MODULES)
	cp $(MODULES) $(BUILD_PATH)/boot/mod

packages: $(BUILD_PATH) $(BINS) $(CONFS) $(SHARES)
	cp $(BINS) $(BUILD_PATH)/bin
	cp $(CONFS) $(BUILD_PATH)/config
	cp $(SHARES) $(BUILD_PATH)/share

ramdisk: $(RAMDISK)

