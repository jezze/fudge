CONFIG:=config
KERNEL:=fudge
RAMDISK_TYPE:=cpio
RAMDISK:=$(KERNEL).$(RAMDISK_TYPE)
RAMDISK_DATA:=alfi config font fudge.pcx help.txt keys motd.txt sh
INIT=init
ISO_TYPE:=iso
ISO=$(KERNEL).$(ISO_TYPE)
DIR_BUILD:=build
DIR_BUILDBOOT:=$(DIR_BUILD)/boot
DIR_BUILDINITRD:=$(DIR_BUILD)/initrd
DIR_BUILDROOT:=$(DIR_BUILD)/root
DIR_BUILDUEFI:=$(DIR_BUILD)/uefi
DIR_INCLUDE:=include
DIR_ISO:=iso
DIR_MK:=mk
DIR_LIB:=lib
DIR_SRC:=src
DIR_INSTALL:=/boot
REPORT:=report.xml

.PHONY: all clean check check-full install help
.SUFFIXES:

all: $(DIR_BUILDBOOT) $(DIR_BUILDROOT) $(DIR_BUILDUEFI)

clean:
	@rm -rf $(DIR_BUILD) $(DIR_ISO) $(KERNEL) $(RAMDISK) $(ISO) $(OBJ) $(DEP) $(LIB) $(BIN) $(KBIN) $(KMAP) $(KMOD) $(REPORT)

check:
	@cppcheck -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) --std=c89 --report-progress --xml --enable=all --check-level=exhaustive --suppress=unusedStructMember --suppress=constParameterPointer --suppress=constVariablePointer --suppress=constParameterCallback --suppress=unusedFunction --suppress=cert-API01-C --suppress=cert-EXP15-C --suppress=cert-STR05-C . 2> $(REPORT)

check-full:
	@cppcheck -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) --std=c89 --report-progress --xml --enable=all --check-level=exhaustive . 2> $(REPORT)

install: $(DIR_INSTALL)/$(KERNEL) $(DIR_INSTALL)/$(RAMDISK) $(DIR_INSTALL)/$(INIT)

arm-integratorcp:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=arm" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=integratorcp" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=arm-unknown-eabi" >> $(DIR_MK)/$(CONFIG).mk

m68k-m68kvirt:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=m68k" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=m68kvirt" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=m68k-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

riscv32-virt:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=riscv32" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=virt" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=riscv32-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

riscv64-virt:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=riscv64" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=virt" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=riscv64-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

x86-mboot:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=x86" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=mboot" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=i386-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

x86-mboot2:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=x86" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=mboot2" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=i386-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

x86_64-mboot2:
	@echo "Writing configuration to $(DIR_MK)/$(CONFIG).mk"
	@echo "ARCH:=x86_64" > $(DIR_MK)/$(CONFIG).mk
	@echo "LOADER:=mboot2" >> $(DIR_MK)/$(CONFIG).mk
	@echo "TARGET:=x86_64-unknown-elf" >> $(DIR_MK)/$(CONFIG).mk

help:
	@echo "Set the configuration first:"
	@echo ""
	@echo "  $$ make <config>"
	@echo ""
	@echo "Where <config> is one of the following:"
	@echo ""
	@echo "  arm-integratorcp"
	@echo "  m68k-m68kvirt"
	@echo "  riscv32-virt"
	@echo "  riscv64-virt"
	@echo "  x86-mboot"
	@echo "  x86-mboot2"
	@echo "  x86_64-mboot2"
	@echo ""
	@echo "Then build and clean:"
	@echo ""
	@echo "  $$ make"
	@echo ""
	@echo "To clean:"
	@echo ""
	@echo "  $$ make clean"
	@echo ""

-include $(DIR_MK)/$(CONFIG).mk

ifeq ($(TARGET),)
ifneq ($(filter-out arm-% m68k-% riscv32-% riscv64-% x86-% x86_64-% help,$(or $(MAKECMDGOALS),all)),)
$(error No configuration in $(DIR_MK)/$(CONFIG).mk, set one with make <config> (make help lists them))
endif
else
include $(DIR_MK)/$(ARCH)-$(LOADER).mk
include $(DIR_LIB)/rules.mk
include $(DIR_SRC)/rules.mk
endif

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
	$(if $(wildcard grub/$(LOADER).cfg),@cp -f grub/$(LOADER).cfg $@/grub/grub.cfg)

$(DIR_BUILDINITRD): $(LIB) $(BIN) $(KMAP) $(KMOD) | $(DIR_BUILD)
	@echo BUILD INITRD $@
	@mkdir -p $@
	@mkdir -p $@/boot
	@mkdir -p $@/lib
	@cp $(LIB) $@/lib
	@mkdir -p $@/bin
	@cp $(BIN) $@/bin
	@mkdir -p $@/data
	@cp -r $(addprefix data/,$(RAMDISK_DATA)) $@/data
	@cp data/config/modules-$(ARCH).mq $@/data/config/modules-arch.mq
	@mkdir -p $@/kernel
	@cp $(KMAP) $@/kernel
	@cp $(KMOD) $@/kernel

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

$(DIR_ISO): $(KERNEL) $(RAMDISK) $(DIR_SRC)/utils/$(INIT)
	@echo ISO $@
	@rm -rf $@
	@mkdir -p $@/bin
	@mkdir -p $@/boot/grub
	@cp $(KERNEL) $@
	@cp $(RAMDISK) $@
	@cp $(DIR_SRC)/utils/$(INIT) $@/bin
	@sed 's|(hd0,3)/bin/|/bin/|' grub/$(LOADER).cfg > $@/boot/grub/grub.cfg

$(KERNEL): $(DIR_SRC)/kernel/$(KERNEL)
	@echo KERNEL $@
	@cp $^ $@

$(KERNEL).tar: $(DIR_BUILDINITRD)
	@echo RAMDISK $@
	@tar -cf $@ $^

$(KERNEL).cpio: $(DIR_BUILDINITRD)
	@echo RAMDISK $@
	@find $^ -depth | cpio -o -H newc > $@

$(KERNEL).iso: $(DIR_ISO)
	@echo ISO $@
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
