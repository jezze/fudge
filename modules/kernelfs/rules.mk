MOD_$(DIR):=$(DIR)/kernelfs.ko.0
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/backend.o $(DIR)/protocol.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
