LIB_$(DIR):=$(DIR)/libarch.a
OBJ_$(DIR):=$(DIR)/arch.o $(DIR)/cpu.o $(DIR)/init.o $(DIR)/kmi.o $(DIR)/reg.o $(DIR)/uart.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
