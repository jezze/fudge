MOD_$(DIR):=$(BUILD_MODULE)/pic.ko
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/pic.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
