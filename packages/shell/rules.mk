BIN_$(DIR):=$(BUILD_BIN)/shell
OBJ_$(DIR):=$(DIR)/shell.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
