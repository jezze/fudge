BIN_$(DIR):=$(DIR)/init
OBJ_$(DIR):=$(DIR)/init.o
CFG_$(DIR):=$(DIR)/init.slang $(DIR)/base.slang $(DIR)/user.slang

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

PACKAGES_BUILD:=$(PACKAGES_BUILD) $(BIN_$(DIR))
PACKAGES_CLEAN:=$(PACKAGES_CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
PACKAGES_CONFIGS:=$(PACKAGES_CONFIGS) $(CFG_$(DIR))

BIN_$(DIR):=$(DIR)/initmod
OBJ_$(DIR):=$(DIR)/initmod.o
CFG_$(DIR):=$(DIR)/initmod.conf

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

PACKAGES_BUILD:=$(PACKAGES_BUILD) $(BIN_$(DIR))
PACKAGES_CLEAN:=$(PACKAGES_CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
PACKAGES_CONFIGS:=$(PACKAGES_CONFIGS) $(CFG_$(DIR))

BIN_$(DIR):=$(DIR)/initfs
OBJ_$(DIR):=$(DIR)/initfs.o
CFG_$(DIR):=$(DIR)/initfs.conf

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

PACKAGES_BUILD:=$(PACKAGES_BUILD) $(BIN_$(DIR))
PACKAGES_CLEAN:=$(PACKAGES_CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
PACKAGES_CONFIGS:=$(PACKAGES_CONFIGS) $(CFG_$(DIR))
