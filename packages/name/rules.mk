BIN_$(DIR):=$(DIR)/named
OBJ_$(DIR):=$(DIR)/named.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))

BIN_$(DIR):=$(DIR)/namef
OBJ_$(DIR):=$(DIR)/namef.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
