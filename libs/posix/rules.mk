LIB_$(DIR):=$(DIR)/libposix.a
OBJ_$(DIR):=$(DIR)/posix.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
