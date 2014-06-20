LIB_$(DIR):=$(DIR)/libarch.a
OBJ_$(DIR):=$(DIR)/arch.o $(DIR)/cpu.o $(DIR)/init.o $(DIR)/kmi.o $(DIR)/reg.o $(DIR)/uart.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS_BUILD:=$(LIBS_BUILD) $(LIB_$(DIR))
LIBS_CLEAN:=$(LIBS_CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
