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
ARCH:=x86
LOADER:=mboot
TARGET:=i386-unknown-elf

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

arm-integratorcp-none:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=arm" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=integratorcp" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=arm-none-eabi" >> $(DIR_MK)/$(CONFIG).mk

arm-integratorcp-unknown:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=arm" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=integratorcp" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=arm-unknown-eabi" >> $(DIR_MK)/$(CONFIG).mk

riscv-medany-elf:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=riscv" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=medany" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=riscv64-elf" >> $(DIR_MK)/$(CONFIG).mk

riscv-medany-linux:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=riscv" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=medany" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=riscv64-linux-gnu" >> $(DIR_MK)/$(CONFIG).mk

riscv-medany-tcc:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=riscv" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=medany" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=riscv64-tcc" >> $(DIR_MK)/$(CONFIG).mk

x86-mboot $(DIR_MK)/$(CONFIG).mk:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=x86" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=mboot" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=i386-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

x86-mboot2:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=x86" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=mboot2" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=i386-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

x86-mboot-tcc:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=x86" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=mboot" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=i386-tcc" >> $(DIR_MK)/$(CONFIG).mk

x86_64-mboot2:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=x86_64" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=mboot2" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=x86_64-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

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
	@echo "  arm-integratorcp-none"
	@echo "  arm-integratorcp-unknown"
	@echo "  arm-integratorcp-tcc"
	@echo "  riscv-medany-elf"
	@echo "  riscv-medany-linux"
	@echo "  riscv-medany-tcc"
	@echo "  x86-mboot (default)"
	@echo "  x86-mboot2"
	@echo "  x86-mboot-tcc"
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
	@cp -f grub/$(LOADER).cfg $@/grub/grub.cfg

$(DIR_BUILDROOT): $(LIB) $(BIN) $(KMAP) $(KMOD) | $(DIR_BUILD)
	@echo BUILD ROOT $@
	@mkdir -p $@
	@mkdir -p $@/boot
	@mkdir -p $@/lib
	@cp $(LIB) $@/lib
	@mkdir -p $@/bin
	@cp $(BIN) $@/bin
	@mkdir -p $@/data
	@cp -r data/* $@/data
	@cp data/config/modules-$(ARCH).mq $@/data/config/modules-arch.mq
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
	@cp grub/$(LOADER).cfg $@/boot/grub

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
