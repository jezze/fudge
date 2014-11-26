BIN_$(DIR):=$(BUILD_BIN)/modload
OBJ_$(DIR):=$(DIR)/modload.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ -lelf $(PACKAGES_LIBS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))

BIN_$(DIR):=$(BUILD_BIN)/modunload
OBJ_$(DIR):=$(DIR)/modunload.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ -lelf $(PACKAGES_LIBS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
