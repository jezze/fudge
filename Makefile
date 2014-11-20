ARCH:=x86

LOADER_x86:=mboot
LOADER_arm:=versatilepb
LOADER:=$(LOADER_$(ARCH))

TARGET_x86:=i386-pc-none-elf
TARGET_arm:=arm-none-eabi
TARGET:=$(TARGET_$(ARCH))

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

ASFLAGS:=-target $(TARGET) -c -nostdlib -O2
CFLAGS:=-target $(TARGET) -msoft-float -c -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2
LDFLAGS:=-target $(TARGET) -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2

ALL:=libs kernel modules packages ramdisk
CLEAN:=$(KERNEL) $(RAMDISK) $(BUILD_PATH)

.PHONY: all clean install $(ALL)

all: $(ALL)

.s.o:
	$(AS) -o $@ $(ASFLAGS) $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $<

$(LIBS_PATH)/%.o: CFLAGS+=-I$(INCLUDE_PATH) -I$(LIBS_PATH)
$(MODULES_PATH)/%.o: CFLAGS+=-I$(INCLUDE_PATH) -I$(LIBS_PATH) -I$(MODULES_PATH)
$(MODULES_PATH)/%.ko.0: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(MODULES_PATH)/%.ko.1: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(MODULES_PATH)/%.ko.2: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(MODULES_PATH)/%.ko.3: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(MODULES_PATH)/%.ko.4: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(MODULES_PATH)/%.ko.5: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(MODULES_PATH)/%.ko.6: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(MODULES_PATH)/%.ko.7: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(MODULES_PATH)/%.ko.8: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(MODULES_PATH)/%.ko.9: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r -L$(BUILD_PATH)/lib -lfudge
$(PACKAGES_PATH)/%.o: CFLAGS+=-I$(INCLUDE_PATH) -I$(LIBS_PATH) -I$(PACKAGES_PATH)
$(PACKAGES_PATH)/%: LDFLAGS+=-L$(BUILD_PATH)/lib -labi -lfudge

DIR:=$(LIBS_PATH)
include $(DIR)/rules.mk
DIR:=$(MODULES_PATH)
include $(DIR)/rules.mk
DIR:=$(PACKAGES_PATH)
include $(DIR)/rules.mk

$(BUILD_PATH):
	mkdir -p $@
	mkdir -p $@/home
	mkdir -p $@/system
	mkdir -p $@/mount
	mkdir -p $@/mount/0
	mkdir -p $@/mount/1
	mkdir -p $@/mount/2
	mkdir -p $@/mount/3
	mkdir -p $@/mount/4
	mkdir -p $@/mount/5
	mkdir -p $@/mount/6
	mkdir -p $@/mount/7

$(BUILD_PATH)/bin: $(BUILD_PATH)
	mkdir -p $@

$(BUILD_PATH)/boot: $(BUILD_PATH)
	mkdir -p $@

$(BUILD_PATH)/boot/mod: $(BUILD_PATH)/boot
	mkdir -p $@

$(BUILD_PATH)/config: $(BUILD_PATH)
	mkdir -p $@

$(BUILD_PATH)/lib: $(BUILD_PATH)
	mkdir -p $@

$(BUILD_PATH)/share: $(BUILD_PATH)
	mkdir -p $@

$(KERNEL_NAME):
	$(LD) -o $@ $(LDFLAGS) -Tlibs/$(LOADER)/linker.ld -L$(BUILD_PATH)/lib -l$(LOADER) -larch -lkernel -lelf -ltar -lcpio -lfudge

$(RAMDISK_NAME).tar: $(BUILD_PATH)
	tar -cf $@ $^

$(RAMDISK_NAME).cpio: $(BUILD_PATH)
	find $^ -depth | cpio -o > $@

clean:
	rm -rf $(CLEAN)

install: $(KERNEL) $(RAMDISK)
	install -m 644 $(KERNEL) $(INSTALL_PATH)
	install -m 644 $(RAMDISK) $(INSTALL_PATH)

kernel: $(KERNEL) $(BUILD_PATH)/boot
	cp $(KERNEL) $(BUILD_PATH)/boot

libs: $(LIBS) $(BUILD_PATH)/lib
	cp $(LIBS) $(BUILD_PATH)/lib

modules: $(MODULES) $(BUILD_PATH)/boot/mod
	cp $(MODULES) $(BUILD_PATH)/boot/mod

packages: $(BINS) $(CONFS) $(SHARES) $(BUILD_PATH)/bin $(BUILD_PATH)/config $(BUILD_PATH)/share
	cp $(BINS) $(BUILD_PATH)/bin
	cp $(CONFS) $(BUILD_PATH)/config
	cp $(SHARES) $(BUILD_PATH)/share

ramdisk: $(RAMDISK)

