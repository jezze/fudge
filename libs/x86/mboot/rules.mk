LIBBOOT:=libs/x86/mboot/libmboot.a
LIBBOOT_OBJ:=libs/x86/mboot/init.o libs/x86/mboot/mboot.o

$(LIBBOOT): $(LIBBOOT_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBBOOT)
LIBS_OBJECTS+=$(LIBBOOT_OBJ)
