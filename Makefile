ARCH:=x86

TARGET_x86:=i386-unknown-elf
TARGET_arm:=arm-unknown-eabi
TARGET:=$(TARGET_$(ARCH))

PLATFORM_x86:=pc
PLATFORM_arm:=integratorcp
PLATFORM:=$(PLATFORM_$(ARCH))

DEFAULT_LIBS_x86:=-lfudge
DEFAULT_LIBS_arm:=-lfudge -lstd
DEFAULT_LIBS:=$(DEFAULT_LIBS_$(ARCH))

LOADER_LIBS_x86:=-lmboot
LOADER_LIBS_arm:=
LOADER_LIBS:=$(LOADER_LIBS_$(ARCH))

INSTALL_PATH:=/boot

INCLUDE_PATH:=include

KERNEL_NAME:=fudge
KERNEL_LIBS:=$(LOADER_LIBS) -larch -lkernel -lelf -ltar -lcpio $(DEFAULT_LIBS)

LIBS_PATH:=libs

MODULES_PATH:=modules
MODULES_LIBS:=$(DEFAULT_LIBS)

PACKAGES_PATH:=packages
PACKAGES_LIBS:=-labi $(DEFAULT_LIBS)

RAMDISK_NAME:=initrd
RAMDISK_TYPE:=tar

BUILD_ROOT:=build
BUILD_BIN:=$(BUILD_ROOT)/bin
BUILD_BOOT:=$(BUILD_ROOT)/boot
BUILD_CONFIG:=$(BUILD_ROOT)/config
BUILD_LIB:=$(BUILD_ROOT)/lib
BUILD_MODULE:=$(BUILD_BOOT)/mod
BUILD_SHARE:=$(BUILD_ROOT)/share

AS:=$(TARGET)-gcc
CC:=$(TARGET)-gcc
LD:=$(TARGET)-gcc

ASFLAGS:=-c -nostdlib -O2
CFLAGS:=-c -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2
LDFLAGS:=-msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2

ALL:=libs kernel modules packages ramdisk
CLEAN:=$(KERNEL_NAME) $(RAMDISK_NAME).$(RAMDISK_TYPE) $(BUILD_ROOT)

.PHONY: all clean install $(ALL)

all: $(ALL)

.s.o:
	$(AS) -o $@ $(ASFLAGS) $<

.c.o:
	$(CC) -o $@ $(CFLAGS) $<

$(LIBS_PATH)/%.o: CFLAGS+=-I$(INCLUDE_PATH) -I$(LIBS_PATH)
$(MODULES_PATH)/%.o: CFLAGS+=-I$(INCLUDE_PATH) -I$(LIBS_PATH) -I$(MODULES_PATH)
$(PACKAGES_PATH)/%.o: CFLAGS+=-I$(INCLUDE_PATH) -I$(LIBS_PATH) -I$(PACKAGES_PATH)

$(BUILD_BIN)/%: LDFLAGS+=-L$(BUILD_LIB)
$(BUILD_MODULE)/%.ko: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.0: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.1: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.2: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.3: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.4: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.5: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.6: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.7: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.8: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r
$(BUILD_MODULE)/%.ko.9: LDFLAGS+=-L$(BUILD_LIB) -T$(MODULES_PATH)/linker.ld -r

DIR:=$(LIBS_PATH)
include $(DIR)/rules.mk
DIR:=$(MODULES_PATH)
include $(DIR)/rules.mk
DIR:=$(PACKAGES_PATH)
include $(DIR)/rules.mk

$(BUILD_ROOT):
	mkdir -p $@
	mkdir -p $@/bin
	mkdir -p $@/boot
	mkdir -p $@/boot/mod
	mkdir -p $@/config
	mkdir -p $@/home
	mkdir -p $@/lib
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

$(BUILD_BOOT)/$(KERNEL_NAME): $(BUILD_ROOT)
	$(LD) -o $@ $(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld -L$(BUILD_ROOT)/lib $(KERNEL_LIBS)

$(KERNEL_NAME): $(BUILD_BOOT)/$(KERNEL_NAME)
	cp $^ $@

$(RAMDISK_NAME).tar: $(BUILD_ROOT)
	tar -cf $@ $^

$(RAMDISK_NAME).cpio: $(BUILD_ROOT)
	find $^ -depth | cpio -o > $@

$(INSTALL_PATH)/$(KERNEL_NAME): $(KERNEL_NAME)
	install -m 644 $^ $@

$(INSTALL_PATH)/$(RAMDISK_NAME).$(RAMDISK_TYPE): $(RAMDISK_NAME).$(RAMDISK_TYPE)
	install -m 644 $^ $@

clean:
	rm -rf $(CLEAN)

install: $(INSTALL_PATH)/$(KERNEL_NAME) $(INSTALL_PATH)/$(RAMDISK_NAME).$(RAMDISK_TYPE)

kernel: libs $(KERNEL_NAME)

libs: $(BUILD_ROOT) $(LIBS)

modules: libs $(BUILD_ROOT) $(MODULES)

packages: libs $(BUILD_ROOT) $(BINS) $(CONFIGS) $(SHARES)
	cp $(CONFIGS) $(BUILD_CONFIG)
	cp $(SHARES) $(BUILD_SHARE)

ramdisk: $(RAMDISK_NAME).$(RAMDISK_TYPE)

