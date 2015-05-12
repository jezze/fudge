BIN_$(DIR):=$(BUILD_BIN)/keymapse
OBJ_$(DIR):=$(DIR)/keymapse.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))

BIN_$(DIR):=$(BUILD_BIN)/keymapus
OBJ_$(DIR):=$(DIR)/keymapus.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
