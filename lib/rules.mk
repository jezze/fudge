include lib/src/arch/rules.$(ARCH).mk

LIB+=lib/libfudge.a
LIB_OBJECTS+=lib/src/elf.o lib/src/flake.o lib/src/memory.o lib/src/string.o

lib/%.o: CCFLAGS+=-Ilib/include

$(LIB): $(LIB_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^
