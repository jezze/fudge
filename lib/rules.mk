LIB+=lib/libfudge.a

LIB_OBJECTS+=lib/src/elf.o
LIB_OBJECTS+=lib/src/gfx.o
LIB_OBJECTS+=lib/src/memory.o
LIB_OBJECTS+=lib/src/posix.o
LIB_OBJECTS+=lib/src/string.o

$(LIB): $(LIB_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

lib/%.o: lib/%.c
	$(CC) -c $(CFLAGS) -Ilib/include -o $@ $<
