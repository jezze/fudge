include modules/src/ext2/rules.mk
include modules/src/log/rules.mk
include modules/src/mbr/rules.mk
include modules/src/nodefs/rules.mk
include modules/src/tty/rules.mk
include modules/src/arch/$(ARCH).mk

CLEAN+=$(MODULES)

modules/%.ko: CCFLAGS+=-Ilib/include -Ikernel/include -Imodules/include
modules/%.ko: LDFLAGS+=-Tmodules/linker.ld -r

modules: $(MODULES)
