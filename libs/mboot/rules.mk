LIBMBOOT+=libs/mboot/libmboot.a

LIBMBOOT_OBJECTS+=libs/mboot/init.o
LIBMBOOT_OBJECTS+=libs/mboot/mboot.o

$(LIBMBOOT): $(LIBMBOOT_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libs/mboot/%.o: libs/mboot/%.c
	$(CC) -c $(CFLAGS) -Ikernel/include -o $@ $<
