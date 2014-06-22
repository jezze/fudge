LIB_$(DIR):=$(DIR)/libcpio.a
OBJ_$(DIR):=$(DIR)/cpio.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
