BIN_$(DIR):=$(DIR)/pcxdata
OBJ_$(DIR):=$(DIR)/pcxdata.o $(PACKAGES_PATH)/gfx/gfx.o $(PACKAGES_PATH)/gfx/pcx.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))

BIN_$(DIR):=$(DIR)/pcxcmap
OBJ_$(DIR):=$(DIR)/pcxcmap.o $(PACKAGES_PATH)/gfx/gfx.o $(PACKAGES_PATH)/gfx/pcx.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
