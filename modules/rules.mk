$(MODULES_PATH)/%.o: CFLAGS+=-I$(LIBS_PATH) -I$(MODULES_PATH)
$(MODULES_PATH)/%.ko: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r

include $(MODULES_PATH)/base/rules.mk
include $(MODULES_PATH)/block/rules.mk
include $(MODULES_PATH)/log/rules.mk
include $(MODULES_PATH)/mtwist/rules.mk
include $(MODULES_PATH)/network/rules.mk
include $(MODULES_PATH)/null/rules.mk
include $(MODULES_PATH)/pipe/rules.mk
include $(MODULES_PATH)/system/rules.mk
include $(MODULES_ARCH_PATH)/rules.mk
