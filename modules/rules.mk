include modules/ext2/rules.mk
include modules/log/rules.mk
include modules/mbr/rules.mk
include modules/nodefs/rules.mk
include modules/tty/rules.mk
include modules/arch/$(ARCH)/rules.mk

modules/%.ko: GCCFLAGS+=-Iinclude

modules: $(BUILD)
