LIBMBOOT:=libs/mboot/libmboot.a
LIBMBOOT_OBJ:=libs/mboot/init.o libs/mboot/mboot.o

$(LIBMBOOT): $(LIBMBOOT_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBMBOOT)
LIBS_OBJECTS+=$(LIBMBOOT_OBJ)
