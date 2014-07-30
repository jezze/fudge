MOD_$(DIR):=$(DIR)/atapi.ko
OBJ_$(DIR):=$(DIR)/main.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
