LIB+=lib/libfudge.a

LIB_OBJECTS+=lib/src/elf.o
LIB_OBJECTS+=lib/src/gfx.o
LIB_OBJECTS+=lib/src/memory.o
LIB_OBJECTS+=lib/src/posix.o
LIB_OBJECTS+=lib/src/string.o

$(LIB): $(LIB_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(LIB_OBJECTS): CCFLAGS+=-Ilib/include
