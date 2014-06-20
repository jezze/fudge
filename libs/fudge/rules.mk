LIB_$(DIR):=$(DIR)/libfudge.a
OBJ_$(DIR):=$(DIR)/ascii.o $(DIR)/memory.o $(DIR)/list.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS_BUILD:=$(LIBS_BUILD) $(LIB_$(DIR))
LIBS_CLEAN:=$(LIBS_CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
