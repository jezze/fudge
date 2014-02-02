LIBABI:=libs/x86/abi/libabi.a
LIBABI_OBJ:=libs/x86/abi/call.o libs/x86/abi/crt0.o

$(LIBABI): $(LIBABI_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBABI)
LIBS_OBJECTS+=$(LIBABI_OBJ)
