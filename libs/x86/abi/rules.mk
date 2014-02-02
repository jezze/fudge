LIBABI:=$(LIBS_PATH)/x86/abi/libabi.a
LIBABI_OBJ:=$(LIBS_PATH)/x86/abi/call.o $(LIBS_PATH)/x86/abi/crt0.o

$(LIBABI): $(LIBABI_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBABI)
LIBS_OBJECTS+=$(LIBABI_OBJ)
