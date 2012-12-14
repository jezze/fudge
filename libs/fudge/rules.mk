LIBFUDGE+=libs/fudge/libfudge.a

LIBFUDGE_OBJECTS+=libs/fudge/src/elf.o
LIBFUDGE_OBJECTS+=libs/fudge/src/gfx.o
LIBFUDGE_OBJECTS+=libs/fudge/src/memory.o
LIBFUDGE_OBJECTS+=libs/fudge/src/posix.o
LIBFUDGE_OBJECTS+=libs/fudge/src/string.o

$(LIBFUDGE): $(LIBFUDGE_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libs/fudge/%.o: libs/fudge/%.c
	$(CC) -c $(CFLAGS) -Ilibs/fudge/include -o $@ $<
