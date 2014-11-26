LIB_$(DIR):=$(BUILD_LIB)/libabi.a
OBJ_$(DIR):=$(DIR)/call.o $(DIR)/crt0.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
