LIBMBOOT=libs/mboot/libmboot.a

LIBMBOOT_OBJECTS+=libs/mboot/init.o
LIBMBOOT_OBJECTS+=libs/mboot/mboot.o

LIBS+=$(LIBMBOOT)
LIBS_OBJECTS+=$(LIBMBOOT_OBJECTS)

$(LIBMBOOT): $(LIBMBOOT_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libs/mboot/%.o: libs/mboot/%.c
	$(CC) -c $(CFLAGS) -Ikernel -o $@ $<
