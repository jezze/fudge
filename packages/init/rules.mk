BIN_$(DIR):=$(BUILD_BIN)/init
OBJ_$(DIR):=$(DIR)/init.o
CONFIG_$(DIR):=$(DIR)/init.slang $(DIR)/user.slang

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(PACKAGES_LIBS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
CONFIGS:=$(CONFIGS) $(CONFIG_$(DIR))

BIN_$(DIR):=$(BUILD_BIN)/initmod
OBJ_$(DIR):=$(DIR)/initmod.o
CONFIG_$(DIR):=$(DIR)/initmod.conf

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(PACKAGES_LIBS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
CONFIGS:=$(CONFIGS) $(CONFIG_$(DIR))

BIN_$(DIR):=$(BUILD_BIN)/initfs
OBJ_$(DIR):=$(DIR)/initfs.o
CONFIG_$(DIR):=$(DIR)/initfs.conf

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(PACKAGES_LIBS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
CONFIGS:=$(CONFIGS) $(CONFIG_$(DIR))
