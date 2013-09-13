modules/%.o: CFLAGS+=-Ilibs -Imodules
modules/%.ko: LDFLAGS+=-Tmodules/linker.ld -r

include modules/base/rules.mk
include modules/block/rules.mk
include modules/log/rules.mk
include modules/mtwist/rules.mk
include modules/net/rules.mk
include modules/null/rules.mk
include modules/system/rules.mk
include modules/temp/rules.mk
include modules/arch/$(ARCH)/rules.mk
