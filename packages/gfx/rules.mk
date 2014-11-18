LIB_$(DIR):=$(DIR)/libgfx.a
OBJ_$(DIR):=$(DIR)/gfx.o $(DIR)/pcx.o $(DIR)/vga.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
