LIB_$(DIR):=$(DIR)/libfudge.a
OBJ_$(DIR):=$(DIR)/ascii.o $(DIR)/memory.o $(DIR)/list.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
