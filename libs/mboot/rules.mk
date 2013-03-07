LIB:=libs/mboot/libmboot.a
OBJ:=libs/mboot/init.o libs/mboot/mboot.o

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBMBOOT:=$(LIB)
LIBS+=$(LIB)
LIBS_OBJECTS+=$(OBJ)
