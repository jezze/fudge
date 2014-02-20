LIBFUDGE:=$(LIBS_PATH)/fudge/libfudge.a
LIBFUDGE_OBJ:=$(LIBS_PATH)/fudge/ascii.o $(LIBS_PATH)/fudge/memory.o $(LIBS_PATH)/fudge/lifo.o $(LIBS_PATH)/fudge/list.o

$(LIBFUDGE): $(LIBFUDGE_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBFUDGE)
LIBS_OBJECTS+=$(LIBFUDGE_OBJ)
