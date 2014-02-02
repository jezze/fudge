LIBABI:=$(LIBS_PATH)/arm/abi/libabi.a
LIBABI_OBJ:=$(LIBS_PATH)/arm/abi/call.o $(LIBS_PATH)/arm/abi/crt0.o

$(LIBABI): $(LIBABI_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBABI)
LIBS_OBJECTS+=$(LIBABI_OBJ)
