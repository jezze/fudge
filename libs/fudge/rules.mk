LIBFUDGE=libs/fudge/libfudge.a

LIBFUDGE_OBJECTS+=libs/fudge/memory.o
LIBFUDGE_OBJECTS+=libs/fudge/posix.o
LIBFUDGE_OBJECTS+=libs/fudge/string.o

include libs/fudge/data/rules.mk
include libs/fudge/format/rules.mk
include libs/fudge/gfx/rules.mk
include libs/fudge/arch/$(ARCH)/rules.mk

LIBS+=$(LIBFUDGE)
LIBS_OBJECTS+=$(LIBFUDGE_OBJECTS)

$(LIBFUDGE): $(LIBFUDGE_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libs/fudge/%.o: libs/fudge/%.c
	$(CC) -c $(CFLAGS) -o $@ $<
