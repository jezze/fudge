BIN_$(DIR):=$(DIR)/complete
OBJ_$(DIR):=$(DIR)/complete.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
