BIN_$(DIR):=$(DIR)/pcxdata
OBJ_$(DIR):=$(DIR)/pcxdata.o $(DIR)/pcx.o $(SRC_PATH)/video/gfx.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
