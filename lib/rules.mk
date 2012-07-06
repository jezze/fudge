include lib/src/arch/rules.$(ARCH).mk

LIBOBJ+=lib/src/elf.o lib/src/flake.o lib/src/memory.o lib/src/string.o

CLEAN+=$(LIBOBJ) lib/libfudge.a

lib/%.o: CCFLAGS+=-Ilib/include

lib/libfudge.a: $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
