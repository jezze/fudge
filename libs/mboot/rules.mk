LIBMBOOT:=libs/mboot/libmboot.a
LIBMBOOT_OBJECTS:=libs/mboot/init.o libs/mboot/mboot.o

$(LIBMBOOT): $(LIBMBOOT_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBMBOOT)
LIBS_OBJECTS+=$(LIBMBOOT_OBJECTS)
