LOADER+=loader/libmboot.a

LOADER_OBJECTS+=loader/mboot/src/init.o
LOADER_OBJECTS+=loader/mboot/src/mboot.o

$(LOADER): $(LOADER_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

loader/mboot/%.o: loader/mboot/%.c
	$(CC) -c $(CFLAGS) -Ilib/include -Ikernel/include -Iloader/mboot/include -o $@ $<
