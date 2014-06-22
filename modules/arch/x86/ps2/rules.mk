MOD_$(DIR):=$(DIR)/ps2.ko
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/keyboard_driver.o $(DIR)/mouse_driver.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) $(LDFLAGS) -o $@ $^

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
