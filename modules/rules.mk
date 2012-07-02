include modules/ext2/rules.mk
include modules/log/rules.mk
include modules/mbr/rules.mk
include modules/nodefs/rules.mk
include modules/tty/rules.mk
include modules/arch/$(ARCH)/rules.mk

CLEAN+=$(MODULES)

modules/%.ko: CCFLAGS+=-Ilib/include -Ikernel/include -Imodules/include
modules/%.ko: LDFLAGS+=-Tmodules/linker.ld -r

modules: $(MODULES)
