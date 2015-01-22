MOD0_$(DIR):=$(BUILD_MODULE)/ethernet.ko.0
MOD1_$(DIR):=$(BUILD_MODULE)/ethernet.ko.1
MOD2_$(DIR):=$(BUILD_MODULE)/ethernet.ko.2
MOD3_$(DIR):=$(BUILD_MODULE)/ethernet.ko.3
MOD4_$(DIR):=$(BUILD_MODULE)/ethernet.ko.4
MOD5_$(DIR):=$(BUILD_MODULE)/ethernet.ko.5
MOD6_$(DIR):=$(BUILD_MODULE)/ethernet.ko.6
MOD7_$(DIR):=$(BUILD_MODULE)/ethernet.ko.7
OBJ_$(DIR):=$(DIR)/main.o

$(MOD0_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

$(MOD1_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

$(MOD2_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

$(MOD3_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

$(MOD4_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

$(MOD5_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

$(MOD6_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

$(MOD7_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(MODULES_LIBS)

MODULES:=$(MODULES) $(MOD0_$(DIR)) $(MOD1_$(DIR)) $(MOD2_$(DIR)) $(MOD3_$(DIR)) $(MOD4_$(DIR)) $(MOD5_$(DIR)) $(MOD6_$(DIR)) $(MOD7_$(DIR))
CLEAN:=$(CLEAN) $(MOD0_$(DIR)) $(MOD1_$(DIR)) $(MOD2_$(DIR)) $(MOD3_$(DIR)) $(MOD4_$(DIR)) $(MOD5_$(DIR)) $(MOD6_$(DIR)) $(MOD7_$(DIR)) $(OBJ_$(DIR))
