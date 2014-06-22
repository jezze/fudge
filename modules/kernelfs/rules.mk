MOD_$(DIR):=$(DIR)/kernelfs.ko
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/backend.o $(DIR)/protocol.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) $(LDFLAGS) -o $@ $^

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
