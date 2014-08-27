MOD_$(DIR):=$(DIR)/io.ko.0
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/io.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
