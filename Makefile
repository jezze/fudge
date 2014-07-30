ARCH:=x86
LOADER:=mboot
TARGET_x86:=i386-pc-none-elf
TARGET_arm:=arm-none-eabi

INSTALL_PATH:=/boot
INCLUDE_PATH:=include
BUILD_PATH:=build
LIBS_PATH:=libs
MODULES_PATH:=modules
PACKAGES_PATH:=packages

KERNEL_NAME:=fudge
KERNEL:=$(KERNEL_NAME)

RAMDISK_NAME:=initrd
RAMDISK_TYPE:=tar
RAMDISK:=$(RAMDISK_NAME).$(RAMDISK_TYPE)

AS:=clang
CC:=clang
LD:=clang

ASFLAGS:=-target $(TARGET_$(ARCH)) -msoft-float -c -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH)
CFLAGS:=-target $(TARGET_$(ARCH)) -msoft-float -c -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH)
LDFLAGS:=-target $(TARGET_$(ARCH)) -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -I$(INCLUDE_PATH) -I$(LIBS_PATH)
ARFLAGS:=rs

.PHONY: all clean install kernel libs modules packages ramdisk

all: kernel libs modules packages ramdisk

.s.o:
	$(AS) -o $@ $(ASFLAGS) $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $<

$(MODULES_PATH)/%.o: CFLAGS+=-I$(MODULES_PATH)
$(MODULES_PATH)/%.ko: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(PACKAGES_PATH)/%: LDFLAGS+=-L$(BUILD_PATH)/lib -labi -lfudge

DIR:=$(LIBS_PATH)
include $(DIR)/rules.mk
DIR:=$(MODULES_PATH)
include $(DIR)/rules.mk
DIR:=$(PACKAGES_PATH)
include $(DIR)/rules.mk

$(KERNEL_NAME): $(BUILD_PATH) libs
	$(LD) -o $@ $(LDFLAGS) -Tlibs/$(ARCH)/$(LOADER)/linker.ld -L$(BUILD_PATH)/lib -lmboot -larch -lkernel -lelf -ltar -lcpio -lfudge

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)
	mkdir -p $(BUILD_PATH)/home
	mkdir -p $(BUILD_PATH)/kernel
	mkdir -p $(BUILD_PATH)/system

$(RAMDISK_NAME).tar: $(BUILD_PATH)
	tar -cf $@ $^

$(RAMDISK_NAME).cpio: $(BUILD_PATH)
	find $^ -depth | cpio -o > $@

clean:
	rm -rf $(BUILD_PATH)
	rm -rf $(CLEAN)
	rm -rf $(KERNEL)
	rm -rf $(RAMDISK)

install: $(KERNEL) $(RAMDISK)
	install -m 644 $(KERNEL) $(INSTALL_PATH)
	install -m 644 $(RAMDISK) $(INSTALL_PATH)

kernel: $(BUILD_PATH) $(KERNEL)
	mkdir -p $(BUILD_PATH)/boot
	cp $(KERNEL) $(BUILD_PATH)/boot

libs: $(BUILD_PATH) $(LIBS)
	mkdir -p $(BUILD_PATH)/lib
	cp $(LIBS) $(BUILD_PATH)/lib

modules: $(BUILD_PATH) $(MODULES)
	mkdir -p $(BUILD_PATH)/boot/mod
	cp $(MODULES) $(BUILD_PATH)/boot/mod

packages: $(BUILD_PATH) libs $(BINS) $(CONFS) $(SHARES)
	mkdir -p $(BUILD_PATH)/bin
	cp $(BINS) $(BUILD_PATH)/bin
	mkdir -p $(BUILD_PATH)/config
	cp $(CONFS) $(BUILD_PATH)/config
	mkdir -p $(BUILD_PATH)/share
	cp $(SHARES) $(BUILD_PATH)/share

ramdisk: $(RAMDISK)

