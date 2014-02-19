LIBFUDGE:=$(LIBS_PATH)/fudge/libfudge.a
LIBFUDGE_OBJ:=$(LIBS_PATH)/fudge/ascii.o $(LIBS_PATH)/fudge/memory.o $(LIBS_PATH)/fudge/list.o

include $(LIBS_PATH)/fudge/data/rules.mk
include $(LIBS_PATH)/fudge/gfx/rules.mk

$(LIBFUDGE): $(LIBFUDGE_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBFUDGE)
LIBS_OBJECTS+=$(LIBFUDGE_OBJ)
