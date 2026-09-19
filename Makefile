CONFIG:=config
KERNEL:=fudge
RAMDISK_TYPE:=cpio
RAMDISK:=$(KERNEL).$(RAMDISK_TYPE)
INIT=init
ISO_TYPE:=iso
ISO=$(KERNEL).$(ISO_TYPE)
DIR_BUILD:=build
DIR_BUILDBOOT:=$(DIR_BUILD)/boot
DIR_BUILDROOT:=$(DIR_BUILD)/root
DIR_BUILDUEFI:=$(DIR_BUILD)/uefi
DIR_INCLUDE:=include
DIR_ISO:=iso
DIR_MK:=mk
DIR_LIB:=lib
DIR_SRC:=src
DIR_INSTALL:=/boot
REPORT:=report.xml

.PHONY: all clean check check-full install default help
.SUFFIXES:

all: $(DIR_BUILDBOOT) $(DIR_BUILDROOT) $(DIR_BUILDUEFI)

clean:
	@rm -rf $(DIR_BUILD) $(DIR_ISO) $(KERNEL) $(RAMDISK) $(ISO) $(OBJ) $(DEP) $(LIB) $(BIN) $(KBIN) $(KMAP) $(KMOD) $(REPORT)

check:
	@cppcheck -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) --std=c89 --report-progress --xml --enable=all --check-level=exhaustive --suppress=unusedStructMember --suppress=constParameterPointer --suppress=constVariablePointer --suppress=constParameterCallback --suppress=unusedFunction --suppress=cert-API01-C --suppress=cert-EXP15-C --suppress=cert-STR05-C . 2> $(REPORT)

check-full:
	@cppcheck -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) --std=c89 --report-progress --xml --enable=all --check-level=exhaustive . 2> $(REPORT)

install: $(DIR_INSTALL)/$(KERNEL) $(DIR_INSTALL)/$(RAMDISK) $(DIR_INSTALL)/$(INIT)

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

config-loader-mboot2:
	@echo "LOADER:=mboot2" >> $(DIR_MK)/$(CONFIG).mk

config-loader-integratorcp:
	@echo "LOADER:=integratorcp" >> $(DIR_MK)/$(CONFIG).mk

config-loader-medany:
	@echo "LOADER:=medany" >> $(DIR_MK)/$(CONFIG).mk

config-target-arm-none-eabi:
	@echo "TARGET:=arm-none-eabi" >> $(DIR_MK)/$(CONFIG).mk

config-target-arm-unknown-eabi:
	@echo "TARGET:=arm-unknown-eabi" >> $(DIR_MK)/$(CONFIG).mk

config-target-i386-unknown-elf:
	@echo "TARGET:=i386-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

config-target-i386-tcc:
	@echo "TARGET:=i386-tcc" >> $(DIR_MK)/$(CONFIG).mk

config-target-riscv64-elf:
	@echo "TARGET:=riscv64-elf" >> $(DIR_MK)/$(CONFIG).mk

config-target-riscv64-linux-gnu:
	@echo "TARGET:=riscv64-linux-gnu" >> $(DIR_MK)/$(CONFIG).mk

config-target-riscv64-tcc:
	@echo "TARGET:=riscv64-tcc" >> $(DIR_MK)/$(CONFIG).mk

config-target-x86_64-unknown-elf:
	@echo "TARGET:=x86_64-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

default: x86-mboot

arm-integratorcp-none: config-init | config-arch-arm config-loader-integratorcp config-target-arm-none-eabi
arm-integratorcp-unknown: config-init | config-arch-arm config-loader-integratorcp config-target-arm-unknown-eabi

riscv-medany-elf: config-init | config-arch-riscv config-loader-medany config-target-riscv64-elf
riscv-medany-linux: config-init | config-arch-riscv config-loader-medany config-target-riscv64-linux-gnu
riscv-medany-tcc: config-init | config-arch-riscv config-loader-medany config-target-riscv64-tcc

x86-mboot: config-init | config-arch-x86 config-loader-mboot config-target-i386-unknown-elf
x86-mboot2: config-init | config-arch-x86 config-loader-mboot2 config-target-i386-unknown-elf
x86-mboot-tcc: config-init | config-arch-x86 config-loader-mboot config-target-i386-tcc

x86_64-mboot: config-init | config-arch-x86 config-loader-mboot config-target-x86_64-unknown-elf
x86_64-mboot2: config-init | config-arch-x86 config-loader-mboot2 config-target-x86_64-unknown-elf

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
	@echo "  arm-integratorcp-none"
	@echo "  arm-integratorcp-unknown"
	@echo "  arm-integratorcp-tcc"
	@echo "  riscv-medany-elf"
	@echo "  riscv-medany-linux"
	@echo "  riscv-medany-tcc"
	@echo "  x86-mboot (default)"
	@echo "  x86-mboot2"
	@echo "  x86-mboot-tcc"
	@echo "  x86_64-mboot"
	@echo "  x86_64-mboot2"
	@echo ""

include $(DIR_MK)/$(CONFIG).mk
include $(DIR_MK)/$(TARGET).mk
include $(DIR_LIB)/rules.mk
include $(DIR_SRC)/rules.mk

deps: $(DEP)

$(DIR_BUILD):
	@echo BUILD $@
	@mkdir -p $@

$(DIR_BUILDBOOT): $(KERNEL) $(RAMDISK) | $(DIR_BUILD)
	@echo BUILD BOOT $@
	@mkdir -p $@
	@cp $(KERNEL) $@
	@cp $(RAMDISK) $@
	@mkdir -p $@/efi
	@mkdir -p $@/grub
	@cp -f grub-$(LOADER).cfg $@/grub/grub.cfg

$(DIR_BUILDROOT): $(LIB) $(BIN) $(KMAP) $(KMOD) | $(DIR_BUILD)
	@echo BUILD ROOT $@
	@mkdir -p $@
	@mkdir -p $@/boot
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

$(DIR_BUILDUEFI):
	@echo BUILD UEFI $@
	@mkdir -p $@
	@mkdir -p $@/EFI/BOOT

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

$(KERNEL).tar: $(DIR_BUILDROOT)
	@echo RAMDISK $@
	@tar -cf $@ $^

$(KERNEL).cpio: $(DIR_BUILDROOT)
	@echo RAMDISK $@
	@find $^ -depth | cpio -o > $@

$(KERNEL).iso: $(DIR_ISO)
	@grub-mkrescue -o $@ $^

$(DIR_INSTALL)/$(KERNEL): $(KERNEL)
	@echo INSTALL $@
	@install -m 644 $^ $@

$(DIR_INSTALL)/$(RAMDISK): $(RAMDISK)
	@echo INSTALL $@
	@install -m 644 $^ $@

$(DIR_INSTALL)/$(INIT): $(DIR_SRC)/utils/$(INIT)
	@echo INSTALL $@
	@install -m 644 $^ $@
