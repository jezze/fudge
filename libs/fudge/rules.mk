LIB:=libs/fudge/libfudge.a
OBJ:=libs/fudge/memory.o libs/fudge/posix.o libs/fudge/string.o

include libs/fudge/data/rules.mk
include libs/fudge/format/rules.mk
include libs/fudge/gfx/rules.mk
include libs/fudge/arch/$(ARCH)/rules.mk

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBFUDGE:=$(LIB)
LIBS+=$(LIB)
LIBS_OBJECTS+=$(OBJ)
