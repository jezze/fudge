include lib/src/arch/rules.$(ARCH).mk

LIB+=lib/libfudge.a

LIBOBJ+=lib/src/elf.o lib/src/flake.o lib/src/memory.o lib/src/string.o

CLEAN+=$(LIBOBJ) $(LIB)

lib/%.o: CCFLAGS+=-Ilib/include

lib/libfudge.a: $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
