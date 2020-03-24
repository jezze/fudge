ARCH:=x86
KERNEL:=fudge
LOADER:=mboot
TARGET:=i386-unknown-elf
RAMDISK_TYPE:=cpio
RAMDISK:=$(KERNEL).$(RAMDISK_TYPE)
IMAGE_TYPE:=img
IMAGE=$(KERNEL).$(IMAGE_TYPE)
ISO_TYPE:=iso
ISO=$(KERNEL).$(ISO_TYPE)
DIR_BUILD:=build
DIR_INCLUDE:=include
DIR_ISO:=iso
DIR_MK:=mk
DIR_LIB:=lib
DIR_SRC:=src
DIR_SNAPSHOT:=snapshot
DIR_INSTALL:=/boot

.PHONY: all clean install
.SUFFIXES:

all: $(KERNEL) $(RAMDISK)

clean:
	@rm -rf $(DIR_BUILD) $(DIR_ISO) $(KERNEL) $(RAMDISK) $(IMAGE) $(ISO) $(OBJ) $(DEP) $(LIB) $(BIN) $(KBIN) $(KMAP) $(KMOD)

install: $(DIR_INSTALL)/$(KERNEL) $(DIR_INSTALL)/$(RAMDISK)

include $(DIR_MK)/$(TARGET).mk
include $(DIR_LIB)/rules.mk
include $(DIR_SRC)/rules.mk

deps: $(DEP)

$(DIR_BUILD): $(LIB) $(BIN) $(KBIN) $(KMAP) $(KMOD)
	@echo BUILDROOT $@
	@mkdir -p $@
	@mkdir -p $@/bin
	@cp $(BIN) $@/bin
	@mkdir -p $@/config
	@cp config/* $@/config
	@mkdir -p $@/data
	@cp data/* $@/data
	@mkdir -p $@/kernel
	@cp $(KBIN) $@/kernel
	@cp $(KMAP) $@/kernel
	@cp $(KMOD) $@/kernel
	@mkdir -p $@/lib
	@cp $(LIB) $@/lib
	@mkdir -p $@/system

$(DIR_ISO): $(KERNEL) $(RAMDISK)
	@echo ISO $@
	@mkdir -p $@
	@mkdir -p $@/boot
	@cp $(KERNEL) $@/boot
	@cp $(RAMDISK) $@/boot
	@mkdir -p $@/boot/grub
	@cp grub.cfg $@/boot/grub

$(KERNEL): $(DIR_SRC)/kernel/$(KERNEL)
	@echo KERNEL $@
	@cp $^ $@

$(KERNEL).tar: $(DIR_BUILD)
	@echo RAMDISK $@
	@tar -cf $@ $^

$(KERNEL).cpio: $(DIR_BUILD)
	@echo RAMDISK $@
	@find $^ -depth | cpio -o > $@

$(KERNEL).img: $(KERNEL) $(RAMDISK)
	@echo IMAGE $@
	@dd if=/dev/zero of=$@ count=65536
	@dd if=$(KERNEL) of=$@ conv=notrunc
	@dd if=$(RAMDISK) of=$@ skip=4096 conv=notrunc

$(KERNEL).iso: $(DIR_ISO)
	@grub-mkrescue -o $@ $^

$(DIR_SNAPSHOT): $(KERNEL) $(RAMDISK)
	@echo SNAPSHOT fudge-`git describe --always`.tar.gz
	@mkdir -p $@
	@md5sum $^ > $@/checksum.md5
	@sha1sum $^ > $@/checksum.sha1
	@cp $^ $@
	@mv $@ fudge-`git describe --always`
	@tar -czf fudge-`git describe --always`.tar.gz fudge-`git describe --always`
	@rm -rf fudge-`git describe --always`

$(DIR_INSTALL)/$(KERNEL): $(KERNEL)
	@echo INSTALL $@
	@install -m 644 $^ $@

$(DIR_INSTALL)/$(RAMDISK): $(RAMDISK)
	@echo INSTALL $@
	@install -m 644 $^ $@
