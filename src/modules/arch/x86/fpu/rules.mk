MOD_$(DIR):=$(DIR)/fpu.ko
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/fpu.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
