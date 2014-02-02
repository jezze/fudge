libs/%.o: CFLAGS+=-Ilibs

include libs/fudge/rules.mk
include libs/kernel/rules.mk
include libs/posix/rules.mk
include libs/$(ARCH)/arch/rules.mk
include libs/$(ARCH)/abi/rules.mk
include libs/$(ARCH)/$(LOADER)/rules.mk
