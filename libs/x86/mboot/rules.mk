LIBLOADER:=libs/x86/mboot/libmboot.a
LIBLOADER_OBJ:=libs/x86/mboot/init.o libs/x86/mboot/mboot.o

$(LIBLOADER): $(LIBLOADER_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBLOADER)
LIBS_OBJECTS+=$(LIBLOADER_OBJ)
