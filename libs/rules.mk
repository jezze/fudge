$(LIBS_PATH)/%.o: CFLAGS+=-I$(LIBS_PATH)

include $(LIBS_PATH)/fudge/rules.mk
include $(LIBS_PATH)/kernel/rules.mk
include $(LIBS_PATH)/posix/rules.mk
include $(LIBS_ARCH_PATH)/arch/rules.mk
include $(LIBS_ARCH_PATH)/abi/rules.mk
include $(LIBS_ARCH_PATH)/$(LOADER)/rules.mk
