include lib/arch/$(ARCH)/rules.mk

LIBOBJ+=lib/elf.o lib/flake.o lib/memory.o lib/string.o
CLEAN+=$(LIBOBJ) lib/libfudge.a

lib/%.o: CCFLAGS+=-Iinclude

lib/libfudge.a: $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
