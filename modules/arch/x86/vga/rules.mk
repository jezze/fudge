MOD_$(DIR):=$(DIR)/vga.ko
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/timing.o $(DIR)/registers.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) $(LDFLAGS) -o $@ $^

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
