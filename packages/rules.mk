include packages/base/rules.mk
include packages/net/rules.mk
include packages/test/rules.mk
include packages/wm/rules.mk

CLEAN+=$(PACKAGESOBJ)

packages: GCCFLAGS+=-Iinclude/lib
packages: lib $(PACKAGESOBJ)
