MOD0_$(DIR):=$(DIR)/pipe.ko.0
MOD1_$(DIR):=$(DIR)/pipe.ko.1
MOD2_$(DIR):=$(DIR)/pipe.ko.2
MOD3_$(DIR):=$(DIR)/pipe.ko.3
MOD4_$(DIR):=$(DIR)/pipe.ko.4
MOD5_$(DIR):=$(DIR)/pipe.ko.5
MOD6_$(DIR):=$(DIR)/pipe.ko.6
MOD7_$(DIR):=$(DIR)/pipe.ko.7
OBJ_$(DIR):=$(DIR)/main.o

$(MOD0_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

$(MOD1_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

$(MOD2_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

$(MOD3_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

$(MOD4_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

$(MOD5_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

$(MOD6_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

$(MOD7_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^

MODULES:=$(MODULES) $(MOD0_$(DIR)) $(MOD1_$(DIR)) $(MOD2_$(DIR)) $(MOD3_$(DIR)) $(MOD4_$(DIR)) $(MOD5_$(DIR)) $(MOD6_$(DIR)) $(MOD7_$(DIR))
CLEAN:=$(CLEAN) $(MOD0_$(DIR)) $(MOD1_$(DIR)) $(MOD2_$(DIR)) $(MOD3_$(DIR)) $(MOD4_$(DIR)) $(MOD5_$(DIR)) $(MOD6_$(DIR)) $(MOD7_$(DIR)) $(OBJ_$(DIR))
