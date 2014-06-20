LIB_$(DIR):=$(DIR)/libposix.a
OBJ_$(DIR):=$(DIR)/posix.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS_BUILD:=$(LIBS_BUILD) $(LIB_$(DIR))
LIBS_CLEAN:=$(LIBS_CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
