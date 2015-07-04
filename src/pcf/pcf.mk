BIN_$(DIR):=$(DIR)/pcf
OBJ_$(DIR):=$(DIR)/pcf.o $(DIR)/lib.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
