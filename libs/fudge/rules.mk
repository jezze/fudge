LIBFUDGE:=libs/fudge/libfudge.a
LIBFUDGE_OBJ:=libs/fudge/memory.o libs/fudge/string.o

include libs/fudge/data/rules.mk
include libs/fudge/format/rules.mk
include libs/fudge/gfx/rules.mk

$(LIBFUDGE): $(LIBFUDGE_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBFUDGE)
LIBS_OBJECTS+=$(LIBFUDGE_OBJ)
