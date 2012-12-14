include libs/rules.$(ARCH).mk

LIBFUDGE+=libs/fudge/libfudge.a

LIBFUDGE_OBJECTS+=libs/fudge/elf.o
LIBFUDGE_OBJECTS+=libs/fudge/gfx.o
LIBFUDGE_OBJECTS+=libs/fudge/memory.o
LIBFUDGE_OBJECTS+=libs/fudge/posix.o
LIBFUDGE_OBJECTS+=libs/fudge/string.o

LIBMBOOT+=libs/mboot/libmboot.a

LIBMBOOT_OBJECTS+=libs/mboot/init.o
LIBMBOOT_OBJECTS+=libs/mboot/mboot.o

$(LIBFUDGE): $(LIBFUDGE_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(LIBMBOOT): $(LIBMBOOT_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libs/fudge/%.o: libs/fudge/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

libs/mboot/%.o: libs/mboot/%.c
	$(CC) -c $(CFLAGS) -Ikernel/include -o $@ $<
