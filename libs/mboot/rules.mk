LIB_$(DIR):=$(BUILD_LIB)/libmboot.a
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/mboot.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
