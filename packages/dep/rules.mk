BIN_$(DIR):=$(BUILD_BIN)/dep
OBJ_$(DIR):=$(DIR)/dep.o $(DIR)/parse.o $(DIR)/token.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(PACKAGES_LDFLAGS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
