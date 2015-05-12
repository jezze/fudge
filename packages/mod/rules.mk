BIN_$(DIR):=$(BUILD_BIN)/modload
OBJ_$(DIR):=$(DIR)/modload.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS) $(LIBS_PATH)/elf/libelf.a
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))

BIN_$(DIR):=$(BUILD_BIN)/modunload
OBJ_$(DIR):=$(DIR)/modunload.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS) $(LIBS_PATH)/elf/libelf.a
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
