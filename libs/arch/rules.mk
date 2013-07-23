LIBARCH:=libs/fudge/libarch.a
LIBARCH_OBJ:=

include libs/arch/$(ARCH)/rules.mk

$(LIBARCH): $(LIBARCH_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBARCH)
LIBS_OBJECTS+=$(LIBARCH_OBJ)
