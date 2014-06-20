BIN_$(DIR):=$(DIR)/pcxdata
OBJ_$(DIR):=$(DIR)/pcxdata.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ -lgfx $(LDFLAGS)

PACKAGES_BUILD:=$(PACKAGES_BUILD) $(BIN_$(DIR))
PACKAGES_CLEAN:=$(PACKAGES_CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))

BIN_$(DIR):=$(DIR)/pcxcmap
OBJ_$(DIR):=$(DIR)/pcxcmap.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ -lgfx $(LDFLAGS)

PACKAGES_BUILD:=$(PACKAGES_BUILD) $(BIN_$(DIR))
PACKAGES_CLEAN:=$(PACKAGES_CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
