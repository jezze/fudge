ARCH:=x86
KERNEL:=fudge
LOADER:=mboot

RAMDISK_NAME:=$(KERNEL)
RAMDISK_TYPE:=cpio
RAMDISK:=$(RAMDISK_NAME).$(RAMDISK_TYPE)

IMAGE_NAME:=$(KERNEL)
IMAGE_TYPE:=img
IMAGE=$(IMAGE_NAME).$(IMAGE_TYPE)

DIR_BUILD:=build
DIR_INCLUDE:=include
DIR_MK:=mk
DIR_LIB:=lib
DIR_SRC:=src
DIR_SNAPSHOT:=snapshot
DIR_INSTALL:=/boot

.PHONY: all clean snapshot install
.SUFFIXES:

all: $(KERNEL) $(RAMDISK)

clean:
	@rm -rf $(DIR_BUILD) $(DIR_SNAPSHOT) $(KERNEL) $(RAMDISK) $(IMAGE) $(OBJ) $(LIB) $(BIN) $(KBIN) $(KMAP) $(KMOD)

snapshot: $(DIR_SNAPSHOT)/latest

install: $(DIR_INSTALL)/$(KERNEL) $(DIR_INSTALL)/$(RAMDISK)

include $(DIR_MK)/arch.$(ARCH).gcc.mk
include $(DIR_LIB)/rules.mk
include $(DIR_SRC)/rules.mk

$(DIR_BUILD): $(BIN) $(KBIN) $(KMAP) $(KMOD)
	@echo BUILDROOT $@
	@mkdir -p $@
	@mkdir -p $@/bin
	@cp $(BIN) $@/bin
	@cp $(KBIN) $@/bin
	@mkdir -p $@/mod
	@cp $(KMAP) $@/mod
	@cp $(KMOD) $@/mod
	@mkdir -p $@/config
	@cp config/* $@/config
	@mkdir -p $@/home
	@mkdir -p $@/mount
	@mkdir -p $@/mount/0
	@mkdir -p $@/mount/1
	@mkdir -p $@/mount/2
	@mkdir -p $@/mount/3
	@mkdir -p $@/mount/4
	@mkdir -p $@/mount/5
	@mkdir -p $@/mount/6
	@mkdir -p $@/mount/7
	@mkdir -p $@/data
	@cp data/* $@/data
	@mkdir -p $@/system

$(KERNEL): $(DIR_SRC)/kernel/$(KERNEL)
	@echo KERNEL $@
	@cp $^ $@

$(RAMDISK_NAME).tar: $(DIR_BUILD)
	@echo RAMDISK $@
	@tar -cf $@ $^

$(RAMDISK_NAME).cpio: $(DIR_BUILD)
	@echo RAMDISK $@
	@find $^ -depth | cpio -o > $@

$(IMAGE_NAME).img: $(KERNEL) $(RAMDISK)
	@echo IMAGE $@
	@dd if=/dev/zero of=$@ count=65536
	@dd if=$(KERNEL) of=$@ conv=notrunc
	@dd if=$(RAMDISK) of=$@ skip=4096 conv=notrunc

$(DIR_SNAPSHOT)/latest: $(KERNEL) $(RAMDISK)
	@echo SNAPSHOT $(DIR_SNAPSHOT)/`git describe --always`
	@mkdir -p $@
	@md5sum $^ > $@/checksum.md5
	@sha1sum $^ > $@/checksum.sha1
	@cp $^ $@
	@mv $@ $(DIR_SNAPSHOT)/`git describe --always`

$(DIR_INSTALL)/$(KERNEL): $(KERNEL)
	@echo INSTALL $@
	@install -m 644 $^ $@

$(DIR_INSTALL)/$(RAMDISK): $(RAMDISK)
	@echo INSTALL $@
	@install -m 644 $^ $@
