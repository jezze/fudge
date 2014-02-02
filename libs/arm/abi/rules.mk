LIBABI:=libs/arm/abi/libabi.a
LIBABI_OBJ:=libs/arm/abi/call.o libs/arm/abi/crt0.o

$(LIBABI): $(LIBABI_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBABI)
LIBS_OBJECTS+=$(LIBABI_OBJ)
