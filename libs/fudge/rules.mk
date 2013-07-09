LIBFUDGE:=libs/fudge/libfudge.a
LIBFUDGE_OBJECTS:=libs/fudge/memory.o libs/fudge/string.o

include libs/fudge/data/rules.mk
include libs/fudge/format/rules.mk
include libs/fudge/gfx/rules.mk
include libs/fudge/arch/$(ARCH)/rules.mk

$(LIBFUDGE): $(LIBFUDGE_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBFUDGE)
LIBS_OBJECTS+=$(LIBFUDGE_OBJECTS)
