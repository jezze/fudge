LIB_$(DIR):=$(DIR)/libutf8.a
OBJ_$(DIR):=$(DIR)/utf8.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS_BUILD:=$(LIBS_BUILD) $(LIB_$(DIR))
LIBS_CLEAN:=$(LIBS_CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
