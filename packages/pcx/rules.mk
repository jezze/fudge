BIN_$(DIR):=$(BUILD_BIN)/pcxdata
OBJ_$(DIR):=$(DIR)/pcxdata.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ -lgfx $(PACKAGES_LDFLAGS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))

BIN_$(DIR):=$(BUILD_BIN)/pcxcmap
OBJ_$(DIR):=$(DIR)/pcxcmap.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ -lgfx $(PACKAGES_LDFLAGS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
