LIB_$(DIR):=$(DIR)/libgfx.a
OBJ_$(DIR):=$(DIR)/gfx.o $(DIR)/pcx.o $(DIR)/vga.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS_BUILD:=$(LIBS_BUILD) $(LIB_$(DIR))
LIBS_CLEAN:=$(LIBS_CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
