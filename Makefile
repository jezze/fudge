CONFIG:=config
KERNEL:=fudge
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
REPORT:=report.xml

.PHONY: all clean check check-full install default help
.SUFFIXES:

all: $(KERNEL) $(RAMDISK)

clean:
	@rm -rf $(DIR_BUILD) $(DIR_ISO) $(KERNEL) $(RAMDISK) $(IMAGE) $(ISO) $(OBJ) $(DEP) $(LIB) $(BIN) $(KBIN) $(KMAP) $(KMOD) $(REPORT)

check:
	@cppcheck -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) --std=c89 --report-progress --xml --enable=all --suppress=unusedStructMember --suppress=constParameterPointer --suppress=constVariablePointer --suppress=constParameterCallback --suppress=unusedFunction --suppress=cert-API01-C --suppress=cert-EXP15-C --suppress=cert-STR05-C . 2> $(REPORT)

check-full:
	@cppcheck -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) --std=c89 --report-progress --xml --enable=all . 2> $(REPORT)

install: $(DIR_INSTALL)/$(KERNEL) $(DIR_INSTALL)/$(RAMDISK)

config-init:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo -n "" > $(DIR_MK)/$(CONFIG).mk

config-arch-x86:
	@echo "ARCH:=x86" >> $(DIR_MK)/$(CONFIG).mk

config-arch-arm:
	@echo "ARCH:=arm" >> $(DIR_MK)/$(CONFIG).mk

config-arch-riscv:
	@echo "ARCH:=riscv" >> $(DIR_MK)/$(CONFIG).mk

config-loader-mboot:
	@echo "LOADER:=mboot" >> $(DIR_MK)/$(CONFIG).mk

config-loader-integratorcp:
	@echo "LOADER:=integratorcp" >> $(DIR_MK)/$(CONFIG).mk

config-loader-medany:
	@echo "LOADER:=medany" >> $(DIR_MK)/$(CONFIG).mk

config-target-i386-unknown-elf:
	@echo "TARGET:=i386-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

config-target-i386-tcc:
	@echo "TARGET:=i386-tcc" >> $(DIR_MK)/$(CONFIG).mk

config-target-arm-none-eabi:
	@echo "TARGET:=arm-none-eabi" >> $(DIR_MK)/$(CONFIG).mk

config-target-arm-unknown-eabi:
	@echo "TARGET:=arm-unknown-eabi" >> $(DIR_MK)/$(CONFIG).mk

config-target-riscv64-linux-gnu:
	@echo "TARGET:=riscv64-linux-gnu" >> $(DIR_MK)/$(CONFIG).mk

config-target-riscv64-tcc:
	@echo "TARGET:=riscv64-tcc" >> $(DIR_MK)/$(CONFIG).mk

x86-mboot: config-init | config-arch-x86 config-loader-mboot config-target-i386-unknown-elf
x86-mboot-tcc: config-init | config-arch-x86 config-loader-mboot config-target-i386-tcc

arm-integratorcp-none: config-init | config-arch-arm config-loader-integratorcp config-target-arm-none-eabi
arm-integratorcp-unknown: config-init | config-arch-arm config-loader-integratorcp config-target-arm-unknown-eabi

riscv-medany-linux: config-init | config-arch-riscv config-loader-medany config-target-riscv64-linux-gnu
riscv-medany-tcc: config-init | config-arch-riscv config-loader-medany config-target-riscv64-tcc

default: x86-mboot

help:
	@echo "Building and cleaning:"
	@echo ""
	@echo "  $$ make"
	@echo "  $$ make clean"
	@echo ""
	@echo "Set configuration (mostly for cross-compiling):"
	@echo ""
	@echo "  $$ make <config>"
	@echo ""
	@echo "Where <config> is one of the following:"
	@echo ""
	@echo "  x86-mboot (default)"
	@echo "  x86-mboot-tcc"
	@echo "  arm-integratorcp-none"
	@echo "  arm-integratorcp-unknown"
	@echo "  arm-integratorcp-tcc"
	@echo "  riscv-medany-linux"
	@echo "  riscv-medany-tcc"
	@echo ""

include $(DIR_MK)/$(CONFIG).mk
include $(DIR_MK)/$(TARGET).mk
include $(DIR_LIB)/rules.mk
include $(DIR_SRC)/rules.mk

deps: $(DEP)

$(DIR_BUILD): $(LIB) $(BIN) $(KMAP) $(KMOD)
	@echo BUILDROOT $@
	@mkdir -p $@
	@mkdir -p $@/lib
	@cp $(LIB) $@/lib
	@mkdir -p $@/bin
	@cp $(BIN) $@/bin
	@mkdir -p $@/config
	@cp config/* $@/config
	@mkdir -p $@/data
	@cp -r data/* $@/data
	@mkdir -p $@/kernel
	@cp $(KMAP) $@/kernel
	@cp $(KMOD) $@/kernel

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
