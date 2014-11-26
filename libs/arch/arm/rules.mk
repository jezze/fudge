LIB_$(DIR):=$(BUILD_LIB)/libarch.a
OBJ_$(DIR):=$(DIR)/arch.o $(DIR)/cpu.o $(DIR)/init.o $(DIR)/kmi.o $(DIR)/pic.o $(DIR)/reg.o $(DIR)/uart.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
