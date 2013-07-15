LIBPOSIX:=libs/posix/libposix.a
LIBPOSIX_OBJ:=libs/posix/posix.o

$(LIBPOSIX): $(LIBPOSIX_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBPOSIX)
LIBS_OBJECTS+=$(LIBPOSIX_OBJ)
