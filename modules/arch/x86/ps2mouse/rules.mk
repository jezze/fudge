MOD_$(DIR):=$(DIR)/ps2mouse.ko.0
OBJ_$(DIR):=$(DIR)/main.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
