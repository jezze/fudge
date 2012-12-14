LIBMBOOT+=libs/mboot/libmboot.a

LIBMBOOT_OBJECTS+=libs/mboot/src/init.o
LIBMBOOT_OBJECTS+=libs/mboot/src/mboot.o

$(LIBMBOOT): $(LIBFUDGE) $(LIBMBOOT_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libs/mboot/%.o: libs/mboot/%.c
	$(CC) -c $(CFLAGS) -Ilibs/fudge/include -Ikernel/include -Ilibs/mboot/include -o $@ $<
