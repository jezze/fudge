include lib/arch/$(ARCH)/rules.mk

LIBOBJ+=lib/elf.o lib/flake.o lib/memory.o lib/string.o
CLEAN+=$(LIBOBJ) lib/libfudge.a

lib: GCCFLAGS+=-Iinclude
lib: $(LIBOBJ)
	$(AR) $(ARFLAGS) lib/libfudge.a $(LIBOBJ)
