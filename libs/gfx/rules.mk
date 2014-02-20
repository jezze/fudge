LIBGFX:=$(LIBS_PATH)/gfx/libgfx.a
LIBGFX_OBJ:=$(LIBS_PATH)/gfx/gfx.o $(LIBS_PATH)/gfx/pcx.o $(LIBS_PATH)/gfx/vga.o

$(LIBGFX): $(LIBGFX_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBGFX)
LIBS_OBJECTS+=$(LIBGFX_OBJ)
