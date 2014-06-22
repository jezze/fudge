MOD_$(DIR):=$(DIR)/ipv4.ko
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/protocol.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) $(LDFLAGS) -o $@ $^

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
