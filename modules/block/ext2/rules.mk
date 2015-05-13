MOD_$(DIR):=$(DIR)/ext2.ko
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/filesystem.o $(DIR)/protocol.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
