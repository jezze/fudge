$(MODULES_PATH)/%.o: CFLAGS+=-I$(MODULES_PATH)
$(MODULES_PATH)/%.ko: LDFLAGS+=-T$(MODULES_PATH)/linker.ld -r

CUR:=$(DIR)
DIR:=$(CUR)/base
include $(DIR)/rules.mk
DIR:=$(CUR)/block
include $(DIR)/rules.mk
DIR:=$(CUR)/log
include $(DIR)/rules.mk
DIR:=$(CUR)/mtwist
include $(DIR)/rules.mk
DIR:=$(CUR)/network
include $(DIR)/rules.mk
DIR:=$(CUR)/null
include $(DIR)/rules.mk
DIR:=$(CUR)/pipe
include $(DIR)/rules.mk
DIR:=$(CUR)/system
include $(DIR)/rules.mk
DIR:=$(CUR)/kernelfs
include $(DIR)/rules.mk
DIR:=$(CUR)/arch/$(ARCH)
include $(DIR)/rules.mk
