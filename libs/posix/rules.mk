LIB:=libs/posix/libposix.a
OBJ:=libs/posix/posix.o

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBPOSIX:=$(LIB)
LIBS+=$(LIB)
LIBS_OBJECTS+=$(OBJ)
