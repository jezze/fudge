LIB:=libs/mboot/libmboot.a
OBJ:=
OBJ+=libs/mboot/init.o
OBJ+=libs/mboot/mboot.o

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBMBOOT:=$(LIB)
LIBS+=$(LIB)
LIBS_OBJECTS+=$(OBJ)
