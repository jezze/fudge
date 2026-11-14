ifneq ($(filter i386-unknown-elf x86_64-unknown-elf,$(TARGET)),)
include $(DIR_SRC)/demo/rules.mk
endif
ifneq ($(filter i386-unknown-elf x86_64-unknown-elf riscv32-unknown-elf riscv64-unknown-elf m68k-unknown-elf,$(TARGET)),)
include $(DIR_SRC)/doom/rules.mk
endif
include $(DIR_SRC)/gameboy/rules.mk
include $(DIR_SRC)/nes/rules.mk
include $(DIR_SRC)/pop/rules.mk
include $(DIR_SRC)/snes/rules.mk
include $(DIR_SRC)/kernel/rules.mk
include $(DIR_SRC)/modules/rules.mk
include $(firstword $(wildcard $(DIR_SRC)/modules/rules.$(ARCH)-$(LOADER).mk) $(DIR_SRC)/modules/rules.$(ARCH).mk)
include $(DIR_SRC)/test/rules.mk
include $(DIR_SRC)/utils/rules.mk
include $(DIR_SRC)/wm/rules.mk
include $(DIR_SRC)/wutils/rules.mk
