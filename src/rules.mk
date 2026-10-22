ifneq ($(filter i386-unknown-elf x86_64-unknown-elf,$(TARGET)),)
include $(DIR_SRC)/demo/rules.mk
include $(DIR_SRC)/doom/rules.mk
include $(DIR_SRC)/gameboy/rules.mk
include $(DIR_SRC)/pop/rules.mk
endif
include $(DIR_SRC)/kernel/rules.mk
include $(DIR_SRC)/modules/rules.mk
include $(DIR_SRC)/modules/rules.$(ARCH).mk
include $(DIR_SRC)/test/rules.mk
include $(DIR_SRC)/utils/rules.mk
include $(DIR_SRC)/wm/rules.mk
include $(DIR_SRC)/wutils/rules.mk
