ifeq ($(TARGET),i386-unknown-elf)
include $(DIR_SRC)/demo/rules.mk
include $(DIR_SRC)/gameboy/rules.mk
endif
include $(DIR_SRC)/kernel/rules.mk
include $(DIR_SRC)/modules/rules.mk
include $(DIR_SRC)/pcx/rules.mk
include $(DIR_SRC)/test/rules.mk
include $(DIR_SRC)/utils/rules.mk
include $(DIR_SRC)/wm/rules.mk
include $(DIR_SRC)/wutils/rules.mk
