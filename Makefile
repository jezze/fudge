ARCH:=x86

TARGET_x86:=i386-unknown-elf
TARGET_arm:=arm-unknown-eabi
TARGET:=$(TARGET_$(ARCH))

PLATFORM_x86:=pc
PLATFORM_arm:=integratorcp
PLATFORM:=$(PLATFORM_$(ARCH))

DEFAULT_LDFLAGS_x86:=-lfudge
DEFAULT_LDFLAGS_arm:=-lfudge -lstd
DEFAULT_LDFLAGS:=$(DEFAULT_LDFLAGS_$(ARCH))

LOADER_LDFLAGS_x86:=-lmboot
LOADER_LDFLAGS_arm:=
LOADER_LDFLAGS:=$(LOADER_LDFLAGS_$(ARCH))

INSTALL_PATH:=/boot

INCLUDE_PATH:=include

KERNEL_NAME:=fudge
KERNEL_LDFLAGS:=$(LOADER_LDFLAGS) -larch -lkernel -lelf -ltar -lcpio $(DEFAULT_LDFLAGS)

LIBS_PATH:=libs

MODULES_PATH:=modules
MODULES_LDFLAGS:=$(DEFAULT_LDFLAGS)

PACKAGES_PATH:=packages
PACKAGES_LDFLAGS:=-labi $(DEFAULT_LDFLAGS)

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

ALL:=$(LIBS_PATH) $(MODULES_PATH) $(PACKAGES_PATH) $(KERNEL_NAME) $(RAMDISK_NAME).$(RAMDISK_TYPE)
CLEAN:=$(BUILD_ROOT) $(KERNEL_NAME) $(RAMDISK_NAME).$(RAMDISK_TYPE)

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
$(BUILD_BOOT)/$(KERNEL_NAME): LDFLAGS+=-L$(BUILD_LIB) -Tplatform/$(PLATFORM)/linker.ld
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
	cp $(CONFIGS) $@/config
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
	cp $(SHARES) $@/share
	mkdir -p $@/system

$(BUILD_BOOT)/$(KERNEL_NAME): $(BUILD_ROOT) $(LIBS)
	$(LD) -o $@ $(LDFLAGS) $(KERNEL_LDFLAGS)

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

$(LIBS_PATH): $(BUILD_ROOT) $(LIBS)

$(MODULES_PATH): $(BUILD_ROOT) $(LIBS) $(MODULES)

$(PACKAGES_PATH): $(BUILD_ROOT) $(LIBS) $(BINS)

clean:
	rm -rf $(CLEAN)

install: $(INSTALL_PATH)/$(KERNEL_NAME) $(INSTALL_PATH)/$(RAMDISK_NAME).$(RAMDISK_TYPE)
